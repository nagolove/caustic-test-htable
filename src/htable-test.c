// vim: set colorcolumn=85
// vim: fdm=marker

#include "koh_table.h"
#include "munit.h"
#include <assert.h>
#include <math.h>
#include <memory.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static bool verbose_print = false;

static MunitResult test_htable_new_free(
    const MunitParameter params[], void* data
) {
    HTable *table = htable_new(NULL);
    htable_free(table);
    return MUNIT_OK;
}

struct NODE {
    int     value;
    bool    not_empty, iterated;
};

struct HashArray {
    struct NODE *arr;
    int         len;
};

struct IterCtx {
    HTable              *table;
    struct HashArray    *harray;
};

HTableAction iter_table(
    const void *key, int key_len, void *value, int value_len, void *udata
) {
    struct IterCtx *ctx = udata;
    // поиск значения по ключу в целевой таблице
    int *value_get_table = htable_get(
        //ctx->table, key, sizeof(ctx->harray->arr[0]), NULL
        ctx->table, key, sizeof(int), NULL
    );

    if (verbose_print) {
        if (value_get_table) 
            printf("iter_table: value_get_table %d\n", *value_get_table);
        else
            printf("iter_table: value_get_table is NULL\n");
    }

    // указатель должен существовать
    // TODO: Проверка со случаями не существующий ключей, на ложные 
    // положительные срабатывания
    munit_assert_not_null(value_get_table);

    munit_assert(ctx->harray->arr[*(int*)key].not_empty == true);
    // получаю значение из хэш-массива
    int value_get_harray = ctx->harray->arr[*(int*)key].value;
    ctx->harray->arr[*(int*)key].iterated = true;
    // должны совпасть
    munit_assert(*value_get_table == value_get_harray);
    return HTABLE_ACTION_NEXT;
}

// TODO: Тот же тест для гетерогенной таблицы, в которой ключи разной длины,
// значения разной длины
static MunitResult test_htable_add_get_each(
    const MunitParameter params[], void* data
) {
    HTable *table = htable_new(NULL);
    struct HashArray harray = {};
    harray.len = 10000;
    harray.arr = calloc(harray.len, sizeof(harray.arr[0]));

    const int hash_items_num = 9000;
    int k = 0;
    while (k < hash_items_num) {
        int key = rand() % harray.len;
        if (!harray.arr[key].not_empty) {
            int value = rand() % INT_MAX;
            harray.arr[key].value = value;
            harray.arr[key].not_empty = true;
            htable_add(table, &key, sizeof(key), &value, sizeof(value));
            k++;
        }
    }

    struct IterCtx ctx = {
        .harray = &harray,
        .table = table,
    };

    // Сравнение с массивом
    htable_each(table, iter_table, &ctx);

    // Проверка количества элементов
    munit_assert_int(htable_count(table), ==, hash_items_num);

    for (int i = 0; i < harray.len; i++) {
        if (harray.arr[i].not_empty) {
            munit_assert(harray.arr[i].iterated == true);
        }
    }

    free(harray.arr);
    htable_free(table);
    return MUNIT_OK;
}

static MunitTest test_suite_tests[] = {

  {
    (char*) "/test_htable_new_free",
    test_htable_new_free,
    NULL,
    NULL,
    MUNIT_TEST_OPTION_NONE,
    NULL
  },

  {
    (char*) "/test_htable_add_get_each",
    test_htable_add_get_each,
    NULL,
    NULL,
    MUNIT_TEST_OPTION_NONE,
    NULL
  },

  { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

static const MunitSuite test_suite = {
  (char*) "htable", test_suite_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE
};

int main(int argc, char **argv) {
    koh_hashers_init();
    return munit_suite_main(&test_suite, (void*) "µnit", argc, argv);
}
