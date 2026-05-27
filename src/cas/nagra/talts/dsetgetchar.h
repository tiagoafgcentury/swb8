
#ifndef _D_SETGETCHAR_H_
#define _D_SETGETCHAR_H_
/*The partner can register their own getchar() implementation with test suite (by using tsSetGetchar API) for the selection of menu or test cases. In this way the menu loop issue will be resolved. If partner not register their API, the test suite will use standard library getchar() for menu/test case selection. */

/*Prototype for partner getchar callback funtion */
typedef int (*ITsGetchar)( void );

/* The partner app can register with their own getchar function as callback in the test suite*/
void tsSetGetchar ( ITsGetchar  xGetchar);
#endif
