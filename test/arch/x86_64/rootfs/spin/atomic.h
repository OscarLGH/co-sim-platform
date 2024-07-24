static inline long atomic_test_and_set(long *p, long val)
{
	asm volatile (
		".byte 0xf2 \n\t"
		"lock xchgq %2, (%1) \n\t"
		"mov %2, %0 \n\t"
		: "=r"(val)
		: "r" (p), "r"(val)
		:
	);
	return val;
}

static inline void atomic_set(long *p, long val)
{
	asm volatile (
		".byte 0xf3 \n\t"
		"movq %1, (%0) \n\t"
		:
		: "r"(p), "r"(val)
		:
	);	
}

static inline long atomic_xadd(long *p, long val)
{
	asm volatile (
		"mov %2, %%rax \n\t"
		".byte 0xf2 \n\t"
		"lock xaddq %%rax, (%1) \n\t"
		"mov %%rax, %0 \n\t"
		: "=r"(val)
		: "r" (p), "r"(val)
		: "rax"
	);
	return val;
}

static inline void atomic_inc(long *p)
{
	asm volatile (
		"lock incq (%0) \n\t"
		:
		:"r"(p)
		:
	);	
}


static inline long atomic_cmpxchg(long *p, long old_val, long new_val)
{
	long ret = 0;
	asm volatile (
		"mov %2, %%rax \n\t"
		"lock cmpxchg %3, (%1) \n\t"
		"mov %%rax, %0 \n\t"
		:"=r" (ret)
		:"r"(p), "r" (old_val), "r"(new_val)
		:"rax"
	);
	return ret;	
}

