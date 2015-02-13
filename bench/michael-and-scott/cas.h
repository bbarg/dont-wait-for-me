
int
cas(void *loc, uint32_t expval, uint32_t newval)
{
  __asm__ __volatile__ (
			"lock\n"
			"cmpxchg %0 %2"		     /* ? how do I refer to inputs */
			: "m" (loc)		     /* outputs */
			: "a" (expval), "d" (newval) /* inputs */
			: "memory", "cc"	     /* clobbers */
			);

  
}
