#include <ust/marker.h>

extern myfunc(void);

int main(void)
{
	myfunc();
	trace_mark(in_prog, MARK_NOARGS);
	return 0;
}
