#include "gtest/gtest.h"
#ifdef __cplusplus
extern "C" {
#endif
#include "../../../utilities/source/list_generic.c"
#ifdef __cplusplus
}
#endif

/**
 * @brief Unit test of list utility.
 *
 * All tests that verify behavior of list utility
 *
 * @file task_list_tests.cpp
 * @author  Jacek Skowronek
 * @date    07/11/2020
 */


using namespace ::testing;

/**
 * @test Adding and removing items from list
 */
TEST(task_list_tests, adding_to_list)
{
	L_LIST* test_subject = list_init();

	uint8_t item1 = 1;
	uint8_t item2 = 1;
	uint8_t item3 = 1;
	uint8_t item4 = 1;
	uint8_t item5 = 1;
	uint8_t item6 = 1;
	uint8_t item7 = 1;

	/**
	 * @<b>scenario<\b>: List is empty, item added.
	 * @<b>expected<\b>: Item added correctly.
	 */
	EXPECT_EQ(RETURN_OK, list_add(test_subject, (void*)&item1));
	EXPECT_EQ(test_subject->size, 1U);
	EXPECT_EQ(test_subject->head->data, (void*)&item1);

	/**
	 * @<b>scenario<\b>: List has 1 element, item is removed.
	 * @<b>expected<\b>: Item removed correctly.
	 */
	EXPECT_EQ(RETURN_OK, list_remove(test_subject, (void*)&item1));
	EXPECT_EQ(test_subject->size, 0U);
	EXPECT_TRUE(test_subject->head == NULL);

	/**
	 * @<b>scenario<\b>: List is empty, 7 item added.
	 * @<b>expected<\b>: Items added correctly.
	 */
	EXPECT_EQ(RETURN_OK, list_add(test_subject, (void*)&item1));
	EXPECT_EQ(RETURN_OK, list_add(test_subject, (void*)&item2));
	EXPECT_EQ(RETURN_OK, list_add(test_subject, (void*)&item3));
	EXPECT_EQ(RETURN_OK, list_add(test_subject, (void*)&item4));
	EXPECT_EQ(RETURN_OK, list_add(test_subject, (void*)&item5));
	EXPECT_EQ(RETURN_OK, list_add(test_subject, (void*)&item6));
	EXPECT_EQ(RETURN_OK, list_add(test_subject, (void*)&item7));
	EXPECT_EQ(test_subject->size, 7U);

	LIST_ITEM* item = test_subject->head;
	EXPECT_EQ(item->data, (void*)&item1);
	item = item->next;
	EXPECT_EQ(item->data, (void*)&item2);
	item = item->next;
	EXPECT_EQ(item->data, (void*)&item3);
	item = item->next;
	EXPECT_EQ(item->data, (void*)&item4);
	item = item->next;
	EXPECT_EQ(item->data, (void*)&item5);
	item = item->next;
	EXPECT_EQ(item->data, (void*)&item6);
	item = item->next;
	EXPECT_EQ(item->data, (void*)&item7);

	/**
	 * @<b>scenario<\b>: List has 7 elements, middle item removed.
	 * @<b>expected<\b>: Item removed correctly.
	 */

	list_remove(test_subject, (void*)&item3);
	item = test_subject->head;
	EXPECT_EQ(item->data, (void*)&item1);
	item = item->next;
	EXPECT_EQ(item->data, (void*)&item2);
	item = item->next;
	EXPECT_EQ(item->data, (void*)&item4);
	item = item->next;
	EXPECT_EQ(item->data, (void*)&item5);
	item = item->next;
	EXPECT_EQ(item->data, (void*)&item6);
	item = item->next;
	EXPECT_EQ(item->data, (void*)&item7);
	EXPECT_EQ(test_subject->size, 6U);

	/**
	 * @<b>scenario<\b>: List has 6 elements, first item removed.
	 * @<b>expected<\b>: Item removed correctly.
	 */
	list_remove(test_subject, (void*)&item1);
	item = test_subject->head;
	EXPECT_EQ(item->data, (void*)&item2);
	item = item->next;
	EXPECT_EQ(item->data, (void*)&item4);
	item = item->next;
	EXPECT_EQ(item->data, (void*)&item5);
	item = item->next;
	EXPECT_EQ(item->data, (void*)&item6);
	item = item->next;
	EXPECT_EQ(item->data, (void*)&item7);
	EXPECT_EQ(test_subject->size, 5U);

	/**
	 * @<b>scenario<\b>: Clearing list.
	 * @<b>expected<\b>: All items removed correctly.
	 */
	list_remove(test_subject, (void*)&item2);
	EXPECT_EQ(test_subject->size, 4U);
	list_remove(test_subject, (void*)&item4);
	EXPECT_EQ(test_subject->size, 3U);
	list_remove(test_subject, (void*)&item5);
	EXPECT_EQ(test_subject->size, 2U);
	list_remove(test_subject, (void*)&item6);
	EXPECT_EQ(test_subject->size, 1U);
	EXPECT_EQ(test_subject->head->data,(void*)&item7);
	list_remove(test_subject, (void*)&item7);
	EXPECT_EQ(test_subject->size, 0U);
	EXPECT_TRUE(test_subject->head == NULL);
}

/**
 * @test Managing two separate list
 */
TEST(task_list_tests, two_list_created)
{
	L_LIST* test_subject = list_init();
	L_LIST* test_subject2 = list_init();

	uint8_t item1 = 1;
	uint8_t item2 = 1;
	uint8_t item3 = 1;
	uint8_t item4 = 1;
	uint8_t item5 = 1;
	uint8_t item6 = 1;
	uint8_t item7 = 1;
	uint8_t item8 = 1;

	EXPECT_EQ(test_subject->size, 0);
	EXPECT_EQ(test_subject2->size, 0);
	EXPECT_TRUE(test_subject->head == NULL);
	EXPECT_TRUE(test_subject2->head == NULL);

	list_add(test_subject, (void*)&item1);
	list_add(test_subject2, (void*)&item5);
	EXPECT_EQ(test_subject->size, 1);
	EXPECT_EQ(test_subject2->size, 1);
	EXPECT_EQ(test_subject->head->data, (void*)&item1);
	EXPECT_EQ(test_subject2->head->data, (void*)&item5);
	EXPECT_TRUE(test_subject->head->next == NULL);
	EXPECT_TRUE(test_subject2->head->next == NULL);

	list_add(test_subject, (void*)&item2);
	list_add(test_subject2, (void*)&item6);
	list_add(test_subject, (void*)&item3);
	list_add(test_subject2, (void*)&item7);
	list_add(test_subject, (void*)&item4);
	list_add(test_subject2, (void*)&item8);

	EXPECT_EQ(test_subject->size, 4);
	EXPECT_EQ(test_subject2->size, 4);
}
