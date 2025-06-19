#include <rte_eal.h>
#include <rte_hash.h>
#include <rte_jhash.h>
#include <rte_random.h>
#include <rte_cycles.h>
#include <inttypes.h>
#include <stdio.h>

#define HASH_ENTRIES  (1 << 20)   // 1 million entries
#define TOTAL_KEYS    (16 * 1024 * 1024) // 16 million keys
#define REPORT_EVERY  (1024 * 1024) // Report every 1 million

static struct rte_hash_parameters hash_params = {
    .name = "cuckoo_test_table",
    .entries = HASH_ENTRIES,
    .key_len = sizeof(uint64_t),
    .hash_func = rte_jhash,
    .hash_func_init_val = 0,
    .extra_flag = RTE_HASH_EXTRA_FLAGS_TRANS_MEM_SUPPORT
};

int main(int argc, char **argv) {
    int ret;
    uint64_t start_time, current_time;
    struct rte_hash *handle;
    
    // Initialize EAL
    ret = rte_eal_init(argc, argv);
    if (ret < 0) {
        fprintf(stderr, "EAL initialization failed: %d\n", ret);
        return -1;
    }

    // Create hash table
    handle = rte_hash_create(&hash_params);
    if (handle == NULL) {
        fprintf(stderr, "Failed to create hash table\n");
        return -1;
    }

    printf("cuckoo test\n");
    start_time = rte_get_tsc_cycles();

    // Calculate nanoseconds per cycle
    double ns_per_cycle = 1e9 / rte_get_tsc_hz();
    
    printf("Time 0 ns\n");
    printf("0 k / %d k, time: 0 ns\n", TOTAL_KEYS / 1024);

    uint64_t special_value = 123456;
    uint64_t special_key = 42;
    
    for (uint64_t i = 1; i <= TOTAL_KEYS; i++) {
        uint64_t key = rte_rand();
        
        if (i == 123456) {
            key = special_key;
            rte_hash_add_key_data(handle, &key, (void *)(uintptr_t)special_value);
        } else {
            rte_hash_add_key_data(handle, &key, (void *)(uintptr_t)i);
        }

        if (i % REPORT_EVERY == 0) {
            current_time = rte_get_tsc_cycles() - start_time;
            uint64_t ns = (uint64_t)(current_time * ns_per_cycle);
            printf("%"PRIu64" k / %d k, time: %"PRIu64" ns\n", 
                   i / 1024, TOTAL_KEYS / 1024, ns);
        }
    }

    uint64_t lookup_start = rte_get_tsc_cycles();
    void *data;
    int32_t pos = rte_hash_lookup_data(handle, &special_key, &data);
    uint64_t lookup_time = (uint64_t)((rte_get_tsc_cycles() - lookup_start) * ns_per_cycle);

    if (pos >= 0) {
        printf("Hash size: %d, key: %"PRIu64", hash idx: %d, data: %"PRIu64", lookup time: %"PRIu64" ns\n",
               HASH_ENTRIES, special_key, pos, (uint64_t)(uintptr_t)data, lookup_time);
    } else {
        printf("Key 42 not found! Error: %d\n", pos);
    }

    rte_hash_free(handle);
    return 0;
}