//============================================================================
// Name        : TNet.cpp
// Author      :
// Version     :
// Copyright   : 3656sec
// Description : 江苏国瑞信安科技有限公司
//============================================================================

#include <iostream>
#include <bson.h>
#include<stdio.h>
#include<time.h>

/*
 * libbson
  /usr/local/include/libbson-1.0
  /usr/local/lib
   bson-1.0
 */

int  test_bson(){
	 bson_t child;
	 bson_t * b_object = bson_new();
	 bson_append_document_begin (b_object, "ping", -1, &child);
	 BSON_APPEND_INT64 (&child, "timestamp", time(NULL));
	 bson_append_document_end (b_object, &child);

	 printf("this is a bson test\n");
	 return 0;
}


void test_create_bson(){
	 bson_t *b;
	   char *j;

	   b = BCON_NEW ("hello", BCON_UTF8 ("bson!"));
	   j = bson_as_json (b, NULL);
	   printf ("%s\n", j);

	   bson_free (j);
	   bson_destroy (b);

}

void test_subarray(){
	bson_t parent;
	bson_t child;
	char *str;
	bson_init (&parent);
	bson_append_document_begin (&parent, "foo", 3, &child);
	bson_append_int32 (&child, "baz", 3, 1);
	bson_append_document_end (&parent, &child);
	str = bson_as_json (&parent, NULL);
	printf ("%s\n", str);
	bson_free (str);

	bson_destroy (&parent);
}

void test_cobject(){
	bson_t *doc;
	char *str;
	doc = BCON_NEW ("foo",
	                "{",
	                "int",
	                BCON_INT32 (1),
	                "array",
	                "[",
	                BCON_INT32 (100),
	                "{",
	                "sub",
	                BCON_UTF8 ("value"),
	                "}",
	                "]",
	                "}");
	str = bson_as_json (doc, NULL);
	printf ("%s\n", str);
	bson_free (str);
	bson_destroy (doc);

}


void test_parse_bson(uint8_t * my_data, size_t my_data_len){
	bson_t *b;
	bson_iter_t iter;

	if ((b = bson_new_from_data (my_data, my_data_len))) {
	   if (bson_iter_init (&iter, b)) {
	      while (bson_iter_next (&iter)) {
	         printf ("Found element key: \"%s\"\n", bson_iter_key (&iter));
	      }
	   }
	   bson_destroy (b);
	}
}

void test_find_field(){
	bson_t *b;
	bson_iter_t iter;
	bson_iter_t baz;
    char* str;
	b =
	   BCON_NEW ("foo", "{", "bar", "[", "{", "baz", BCON_INT32 (1), "}", "]", "}");


	str = bson_as_json(b,NULL);
	printf("json=%s\n",str);
	bson_free(str);

	if (bson_iter_init (&iter, b) &&
	    bson_iter_find_descendant (&iter, "foo.bar.0.baz", &baz) &&
	    BSON_ITER_HOLDS_INT32 (&baz)) {
	   printf ("baz = %d\n", bson_iter_int32 (&baz));
	}

	bson_destroy (b);
}




int  main() {
	test_bson();
	test_subarray();
	test_create_bson();
	test_cobject();
	test_find_field();


   return 0;
}
