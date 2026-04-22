#include <cstdio>
#include <cstring>
#include <cstdlib>

const int HASH_SIZE = 100007;
const int MAX_USERS = 1000;
const int MAX_TRAINS = 1000;
const int MAX_ORDERS = 10000;
const int MAX_STATIONS = 100;
const int MAX_DAYS = 100;

struct StringNode {
    char key[30];
    int val;
    StringNode* next;
};

StringNode* string_pool;
int string_pool_ptr;

StringNode* new_string_node(const char* key, int val) {
    StringNode* node = &string_pool[string_pool_ptr++];
    strcpy(node->key, key);
    node->val = val;
    node->next = nullptr;
    return node;
}

struct HashMap {
    StringNode* buckets[HASH_SIZE];

    void init() {
        for (int i = 0; i < HASH_SIZE; i++) {
            buckets[i] = nullptr;
        }
    }
};

HashMap user_map;
HashMap online_users;
HashMap train_map;

struct User {
    char username[30];
    char password[40];
    char name[20];
    char mailAddr[40];
    int privilege;
    bool online;
};

User users[MAX_USERS];
int user_count;

struct Train {
    char trainID[30];
    int stationNum;
    char stations[MAX_STATIONS][20];
    int seatNum;
    int prices[MAX_STATIONS];
    char startTime[10];
    int travelTimes[MAX_STATIONS];
    int stopoverTimes[MAX_STATIONS];
    int saleDateStart;
    int saleDateEnd;
    char type;
    bool released;
    bool deleted;
    int seats[MAX_DAYS][MAX_STATIONS];
};

Train trains[MAX_TRAINS];
int train_count;

struct Order {
    char username[30];
    char trainID[30];
    char from[20];
    char to[20];
    int date;
    int start_min;
    int end_min;
    int end_day;
    int price;
    int num;
    int train_idx;
    int from_idx;
    int to_idx;
    int status;
    int timestamp;
    bool queue;
};

Order orders[MAX_ORDERS];
int order_count;
Order* user_orders[MAX_USERS][1000];
int user_order_count[MAX_USERS];

int main() {
    printf("Starting main\n");
    fflush(stdout);
    
    string_pool = (StringNode*)malloc(sizeof(StringNode) * HASH_SIZE * 10);
    string_pool_ptr = 0;
    
    printf("Allocated string_pool\n");
    fflush(stdout);
    
    user_map.init();
    online_users.init();
    train_map.init();
    
    for (int i = 0; i < MAX_USERS; i++) {
        user_order_count[i] = 0;
    }
    
    printf("Initialized\n");
    fflush(stdout);
    
    // Simulate adding many users
    for (int i = 0; i < 1000; i++) {
        char username[30];
        sprintf(username, "user%d", i);
        user_map.insert(username, i);
    }
    
    printf("Added 1000 users\n");
    fflush(stdout);
    
    // Simulate adding many trains
    for (int i = 0; i < 1000; i++) {
        char trainID[30];
        sprintf(trainID, "train%d", i);
        train_map.insert(trainID, i);
    }
    
    printf("Added 1000 trains\n");
    fflush(stdout);
    
    // Simulate querying
    for (int i = 0; i < 10000; i++) {
        char username[30];
        sprintf(username, "user%d", i % 1000);
        int idx = user_map.get(username);
    }
    
    printf("Queried 10000 times\n");
    fflush(stdout);
    
    return 0;
}
