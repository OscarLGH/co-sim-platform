#include "ip.hh"
#include "bus.hh"
#include <chrono>

base_ip::base_ip(base_bus *bus, uint64_t id, IP_TYPE type,
            uint64_t base_address, uint64_t size,
            uint64_t irq_vec_start, uint64_t irq_vector_cnt)
{
    LOG_DEBUG("ip constructed.\
        id = %d, type = %d, base_addr = %lx, size = %lx, irq_vec_start = %lx, irq_vector_cnt = %lx.",
        id, type, base_address, size, irq_vec_start, irq_vector_cnt
    );

    this->base_addr = base_address;
    this->addr_size = size;
    this->bus = bus;
    this->id = id;
    this->vector_start = irq_vec_start;
    this->nr_vectors = irq_vector_cnt;
    this->ip_type = type; // Default type, can be set in derived classes
    this->action_thread_running.store(false);
    this->bus->connect_ip(this);
}

void base_ip::mem_master_read(uint64_t addr, uint64_t size, void *data)
{
    if (bus) {
        bus->master_read(addr, size, data);
    } else {
        LOG_ERROR("Error: No bus connected to this IP.");
    }
}

void base_ip::mem_master_write(uint64_t addr, uint64_t size, void *data)
{
    if (bus) {
        bus->master_write(addr, size, data);
    } else {
        LOG_ERROR("Error: No bus connected to this IP.");
    }
}

void base_ip::post_irq(uint64_t id, uint64_t vector)
{
    if (bus) {
        bus->post_irq(id, vector);
    } else {
        LOG_ERROR("Error: No bus connected to this IP for posting IRQ.");
    }
}

void base_ip::trigger_action(const ip_action &action)
{
    std::lock_guard<std::mutex> lock(action_mtx);
    action_queue.push(action);
    action_cv.notify_one();
    LOG_DEBUG("IP %lu triggered action type=%d", id, action.type);
}

void base_ip::start_action_thread()
{
    if (!action_thread_running.load()) {
        action_thread_running.store(true);
        action_thread = std::thread(&base_ip::action_thread_func, this);
        LOG_DEBUG("IP %lu action thread started", id);
    }
}

void base_ip::stop_action_thread()
{
    if (action_thread_running.load()) {
        action_thread_running.store(false);
        action_cv.notify_all();
        if (action_thread.joinable()) {
            action_thread.join();
        }
        LOG_DEBUG("IP %lu action thread stopped", id);
    }
}

void base_ip::action_thread_func()
{
    LOG_DEBUG("IP %lu action thread running", id);
    
    while (action_thread_running.load()) {
        ip_action action;
        
        {
            std::unique_lock<std::mutex> lock(action_mtx);
            // Wait for new actions or shutdown signal
            action_cv.wait(lock, [this] {
                return !action_queue.empty() || !action_thread_running.load();
            });
            
            // Check if we should exit
            if (!action_thread_running.load() && action_queue.empty()) {
                break;
            }
            
            // Get the next action
            if (!action_queue.empty()) {
                action = action_queue.front();
                action_queue.pop();
            }
        }
        
        // Process the action (outside the lock)
        if (action.type != IP_ACTION_NONE) {
            process_action(action);
        }
    }
    
    LOG_DEBUG("IP %lu action thread exiting", id);
}
