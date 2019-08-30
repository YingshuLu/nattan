
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "base64.h"

int main() {

	const char *test = "test i am string";
	int test_len = strlen(test);
	int len = Base64encode_len(test_len);
	char* buf = (char*)malloc(len);

	int n = Base64encode(buf, test, test_len);

	printf("test string: %s, base64: %s, base64 length: %d\n", test, buf, n);

	int new_len = Base64decode_len(buf);
	char *new_buf = (char *) malloc(new_len);

	n = Base64decode(new_buf, buf);

	printf("decoded : %s, len: %d\n", new_buf, n);
 
	return 0;

}
