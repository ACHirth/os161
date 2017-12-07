
/* Auguste Hirth
 * Attempt at adding test to tests menu
 */


#include <types.h>
#include <lib.h>
#include <array.h>
#include <test.h>


int mytest(int a, char** b){
	
	kprintf("%d, %p \n", a, b);
	for(int i = 0; i < a; i++){
		kprintf("%s\n", b[i]);

	}
	return 0;
}
