#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <ctime>

const int MAX_LEN = 256;
const int HASH_SIZE = 100007;
const int MAX_USERS = 1000;
const int MAX_TRAINS = 1000;
const int MAX_ORDERS = 10000;
const int MAX_STATIONS = 100;
const int MAX_DAYS = 100;

unsigned int hash_str(const char* str) {
    unsigned int hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash % HASH_SIZE;
}

int date_to_int(const char* date) {
    int month, day;
    sscanf(date, "%d-%d", &month, &day);
    return (month - 6) * 31 + (day - 1);
}

int time_to_minutes(const char* time) {
    int hour, minute;
    sscanf(time, "%d:%d", &hour, &minute);
    return hour * 60 + minute;
}

void int_to_date(int d, char* buf) {
    int month = 6 + d / 31;
    int day = 1 + d % 31;
    sprintf(buf, "%02d-%02d", month, day);
}

void add_minutes(int base_day, int base_min, int add_min, int& out_day, int& out_min) {
    out_min = base_min + add_min;
    out_day = base_day;
    while (out_min >= 24 * 60) {
        out_min -= 24 * 60;
        out_day++;
    }
}

int str_compare(const char* a, const char* b) {
    return strcmp(a, b);
}

void str_copy(char* dest, const char* src) {
    strcpy(dest, src);
}

int str_len(const char* s) {
    return strlen(s);
}

bool str_equal(const char* a, const char* b) {
    return strcmp(a, b) == 0;
}

struct StringNode {
    char key[30];
    int val;
    StringNode* next;
};

StringNode* string_pool;
int string_pool_ptr;

StringNode* new_string_node(const char* key, int val) {
    StringNode* node = &string_pool[string_pool_ptr++];
    str_copy(node->key, key);
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

    int get(const char* key) {
        unsigned int h = hash_str(key);
        StringNode* node = buckets[h];
        while (node) {
            if (str_equal(node->key, key)) {
                return node->val;
            }
            node = node->next;
        }
        return -1;
    }

    bool insert(const char* key, int val) {
        unsigned int h = hash_str(key);
        StringNode* node = buckets[h];
        while (node) {
            if (str_equal(node->key, key)) {
                return false;
            }
            node = node->next;
        }
        StringNode* new_node = new_string_node(key, val);
        new_node->next = buckets[h];
        buckets[h] = new_node;
        return true;
    }

    bool erase(const char* key) {
        unsigned int h = hash_str(key);
        StringNode* node = buckets[h];
        StringNode* prev = nullptr;
        while (node) {
            if (str_equal(node->key, key)) {
                if (prev) {
                    prev->next = node->next;
                } else {
                    buckets[h] = node->next;
                }
                return true;
            }
            prev = node;
            node = node->next;
        }
        return false;
    }
};

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
HashMap user_map;
HashMap online_users;

bool is_valid_username(const char* s) {
    if (!s || s[0] < 'A' || s[0] > 'Z') return false;
    for (int i = 0; s[i]; i++) {
        char c = s[i];
        if (!((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
              (c >= '0' && c <= '9') || c == '_')) {
            return false;
        }
    }
    return true;
}

bool is_valid_password(const char* s) {
    int len = str_len(s);
    if (len < 6 || len > 30) return false;
    for (int i = 0; s[i]; i++) {
        char c = s[i];
        if (!((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
              (c >= '0' && c <= '9') || c == '_')) {
            return false;
        }
    }
    return true;
}

int add_user(const char* cur_username, const char* username, const char* password,
             const char* name, const char* mailAddr, int privilege) {
    if (user_count > 0) {
        if (!cur_username || cur_username[0] == '\0') return -1;
        int cur_idx = user_map.get(cur_username);
        if (cur_idx < 0) return -1;
        if (!users[cur_idx].online) return -1;
        if (privilege >= users[cur_idx].privilege) return -1;
    } else {
        privilege = 10;
    }

    if (user_map.get(username) >= 0) return -1;

    int idx = user_count++;
    str_copy(users[idx].username, username);
    str_copy(users[idx].password, password);
    str_copy(users[idx].name, name);
    str_copy(users[idx].mailAddr, mailAddr);
    users[idx].privilege = privilege;
    users[idx].online = false;

    user_map.insert(username, idx);
    return 0;
}

int login(const char* username, const char* password) {
    int idx = user_map.get(username);
    if (idx < 0) return -1;
    if (users[idx].online) return -1;
    if (!str_equal(users[idx].password, password)) return -1;

    users[idx].online = true;
    online_users.insert(username, idx);
    return 0;
}

int logout(const char* username) {
    int idx = user_map.get(username);
    if (idx < 0) return -1;
    if (!users[idx].online) return -1;

    users[idx].online = false;
    online_users.erase(username);
    return 0;
}

int query_profile(const char* cur_username, const char* username, char* output) {
    int cur_idx = user_map.get(cur_username);
    if (cur_idx < 0) return -1;
    if (!users[cur_idx].online) return -1;

    int idx = user_map.get(username);
    if (idx < 0) return -1;

    if (cur_idx != idx && users[cur_idx].privilege <= users[idx].privilege) {
        return -1;
    }

    sprintf(output, "%s %s %s %d", users[idx].username, users[idx].name,
            users[idx].mailAddr, users[idx].privilege);
    return 0;
}

int modify_profile(const char* cur_username, const char* username,
                   const char* password, const char* name, const char* mailAddr, int privilege,
                   char* output) {
    int cur_idx = user_map.get(cur_username);
    if (cur_idx < 0) return -1;
    if (!users[cur_idx].online) return -1;

    int idx = user_map.get(username);
    if (idx < 0) return -1;

    if (cur_idx != idx && users[cur_idx].privilege <= users[idx].privilege) {
        return -1;
    }

    if (privilege >= 0 && privilege >= users[cur_idx].privilege) {
        return -1;
    }

    if (password && password[0] != '\0') {
        str_copy(users[idx].password, password);
    }
    if (name && name[0] != '\0') {
        str_copy(users[idx].name, name);
    }
    if (mailAddr && mailAddr[0] != '\0') {
        str_copy(users[idx].mailAddr, mailAddr);
    }
    if (privilege >= 0) {
        users[idx].privilege = privilege;
    }

    sprintf(output, "%s %s %s %d", users[idx].username, users[idx].name,
            users[idx].mailAddr, users[idx].privilege);
    return 0;
}

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
HashMap train_map;

int add_train(const char* trainID, int stationNum, int seatNum,
              const char* stations_str, const char* prices_str,
              const char* startTime, const char* travelTimes_str,
              const char* stopoverTimes_str, const char* saleDate_str, char type) {
    if (train_map.get(trainID) >= 0) return -1;

    int idx = train_count++;
    str_copy(trains[idx].trainID, trainID);
    trains[idx].stationNum = stationNum;
    trains[idx].seatNum = seatNum;
    str_copy(trains[idx].startTime, startTime);
    trains[idx].type = type;
    trains[idx].released = false;
    trains[idx].deleted = false;

    char temp[2000];
    str_copy(temp, stations_str);
    char* token = strtok(temp, "|");
    for (int i = 0; i < stationNum && token; i++) {
        str_copy(trains[idx].stations[i], token);
        token = strtok(nullptr, "|");
    }

    str_copy(temp, prices_str);
    token = strtok(temp, "|");
    trains[idx].prices[0] = 0;
    for (int i = 0; i < stationNum - 1 && token; i++) {
        int price = atoi(token);
        trains[idx].prices[i + 1] = trains[idx].prices[i] + price;
        token = strtok(nullptr, "|");
    }

    str_copy(temp, travelTimes_str);
    token = strtok(temp, "|");
    for (int i = 0; i < stationNum - 1 && token; i++) {
        trains[idx].travelTimes[i] = atoi(token);
        token = strtok(nullptr, "|");
    }

    for (int i = 0; i < stationNum - 2; i++) {
        trains[idx].stopoverTimes[i] = 0;
    }
    if (stationNum > 2) {
        str_copy(temp, stopoverTimes_str);
        token = strtok(temp, "|");
        for (int i = 0; i < stationNum - 2 && token; i++) {
            trains[idx].stopoverTimes[i] = atoi(token);
            token = strtok(nullptr, "|");
        }
    }

    char date1[20], date2[20];
    str_copy(temp, saleDate_str);
    token = strtok(temp, "|");
    str_copy(date1, token);
    token = strtok(nullptr, "|");
    str_copy(date2, token);
    trains[idx].saleDateStart = date_to_int(date1);
    trains[idx].saleDateEnd = date_to_int(date2);

    for (int d = 0; d < MAX_DAYS; d++) {
        for (int s = 0; s < stationNum; s++) {
            trains[idx].seats[d][s] = seatNum;
        }
    }

    train_map.insert(trainID, idx);
    return 0;
}

int release_train(const char* trainID) {
    int idx = train_map.get(trainID);
    if (idx < 0) return -1;
    if (trains[idx].released || trains[idx].deleted) return -1;

    trains[idx].released = true;
    return 0;
}

int delete_train(const char* trainID) {
    int idx = train_map.get(trainID);
    if (idx < 0) return -1;
    if (trains[idx].released || trains[idx].deleted) return -1;

    trains[idx].deleted = true;
    train_map.erase(trainID);
    return 0;
}

int query_train(const char* trainID, int date, char* output) {
    int idx = train_map.get(trainID);
    if (idx < 0) return -1;
    if (trains[idx].deleted) return -1;

    Train& t = trains[idx];
    int start_min = time_to_minutes(t.startTime);

    char* out_ptr = output;
    out_ptr += sprintf(out_ptr, "%s %c\n", t.trainID, t.type);

    for (int i = 0; i < t.stationNum; i++) {
        if (i == 0) {
            out_ptr += sprintf(out_ptr, "%s xx-xx xx:xx -> ", t.stations[i]);
            char date_buf[20];
            int_to_date(date, date_buf);
            out_ptr += sprintf(out_ptr, "%s %02d:%02d 0 %d\n", date_buf, start_min / 60, start_min % 60, t.seats[date][i]);
        } else if (i == t.stationNum - 1) {
            int arrive_day, arrive_min;
            add_minutes(date, start_min, t.travelTimes[i-1], arrive_day, arrive_min);
            char date_buf[20];
            int_to_date(arrive_day, date_buf);
            out_ptr += sprintf(out_ptr, "%s %s %02d:%02d -> xx-xx xx:xx %d x\n",
                        t.stations[i], date_buf, arrive_min / 60, arrive_min % 60,
                        t.prices[i]);
        } else {
            int arrive_day, arrive_min, leave_day, leave_min;
            add_minutes(date, start_min, t.travelTimes[i-1], arrive_day, arrive_min);
            leave_min = arrive_min + t.stopoverTimes[i-1];
            leave_day = arrive_day;
            while (leave_min >= 24 * 60) {
                leave_min -= 24 * 60;
                leave_day++;
            }

            char arrive_date[20], leave_date[20];
            int_to_date(arrive_day, arrive_date);
            int_to_date(leave_day, leave_date);

            out_ptr += sprintf(out_ptr, "%s %s %02d:%02d -> %s %02d:%02d %d %d\n",
                        t.stations[i], arrive_date, arrive_min / 60, arrive_min % 60,
                        leave_date, leave_min / 60, leave_min % 60,
                        t.prices[i], t.seats[date][i]);
        }
    }
    return 0;
}

struct TicketResult {
    int train_idx;
    int from_idx;
    int to_idx;
    int start_day;
    int start_min;
    int end_day;
    int end_min;
    int price;
    int seat;
    int duration;
    int transfer_day;
    int transfer_min;
};

TicketResult ticket_results[10000];
int ticket_result_count;

int get_station_idx(Train& t, const char* station) {
    for (int i = 0; i < t.stationNum; i++) {
        if (str_equal(t.stations[i], station)) {
            return i;
        }
    }
    return -1;
}

void query_ticket(const char* from, const char* to, int date, const char* priority, char* output) {
    ticket_result_count = 0;

    for (int i = 0; i < train_count; i++) {
        if (!trains[i].released || trains[i].deleted) continue;

        Train& t = trains[i];
        int from_idx = get_station_idx(t, from);
        int to_idx = get_station_idx(t, to);

        if (from_idx < 0 || to_idx < 0 || from_idx >= to_idx) continue;

        int start_min = time_to_minutes(t.startTime);

        int arrive_day, arrive_min;
        add_minutes(date, start_min, t.travelTimes[from_idx], arrive_day, arrive_min);

        int end_day, end_min;
        add_minutes(arrive_day, arrive_min, 0, end_day, end_min);
        for (int j = from_idx; j < to_idx - 1; j++) {
            add_minutes(end_day, end_min, t.stopoverTimes[j] + t.travelTimes[j+1], end_day, end_min);
        }

        int price = t.prices[to_idx] - t.prices[from_idx];
        int min_seat = t.seats[date][from_idx];
        for (int j = from_idx; j < to_idx; j++) {
            if (t.seats[date][j] < min_seat) {
                min_seat = t.seats[date][j];
            }
        }

        int duration = (end_day - date) * 24 * 60 + (end_min - start_min);

        TicketResult& r = ticket_results[ticket_result_count++];
        r.train_idx = i;
        r.from_idx = from_idx;
        r.to_idx = to_idx;
        r.start_day = date;
        r.start_min = start_min;
        r.end_day = end_day;
        r.end_min = end_min;
        r.price = price;
        r.seat = min_seat;
        r.duration = duration;
    }

    for (int i = 0; i < ticket_result_count; i++) {
        for (int j = i + 1; j < ticket_result_count; j++) {
            bool swap = false;
            if (str_equal(priority, "time")) {
                if (ticket_results[i].duration > ticket_results[j].duration) swap = true;
                else if (ticket_results[i].duration == ticket_results[j].duration) {
                    if (str_compare(trains[ticket_results[i].train_idx].trainID,
                                   trains[ticket_results[j].train_idx].trainID) > 0) {
                        swap = true;
                    }
                }
            } else {
                if (ticket_results[i].price > ticket_results[j].price) swap = true;
                else if (ticket_results[i].price == ticket_results[j].price) {
                    if (str_compare(trains[ticket_results[i].train_idx].trainID,
                                   trains[ticket_results[j].train_idx].trainID) > 0) {
                        swap = true;
                    }
                }
            }
            if (swap) {
                TicketResult temp = ticket_results[i];
                ticket_results[i] = ticket_results[j];
                ticket_results[j] = temp;
            }
        }
    }

    char* out_ptr = output;
    out_ptr += sprintf(out_ptr, "%d\n", ticket_result_count);

    for (int i = 0; i < ticket_result_count; i++) {
        TicketResult& r = ticket_results[i];
        Train& t = trains[r.train_idx];

        char start_date[20], end_date[20];
        int_to_date(r.start_day, start_date);
        int_to_date(r.end_day, end_date);

        out_ptr += sprintf(out_ptr, "%s %s %s %02d:%02d -> %s %s %02d:%02d %d %d\n",
                    t.trainID, t.stations[r.from_idx], start_date,
                    r.start_min / 60, r.start_min % 60,
                    t.stations[r.to_idx], end_date, r.end_min / 60, r.end_min % 60,
                    r.price, r.seat);
    }
}

TicketResult best_transfer1, best_transfer2;
bool found_transfer;
int best_total_duration;
int best_total_price;

void query_transfer(const char* from, const char* to, int date, const char* priority, char* output) {
    found_transfer = false;
    best_total_duration = 0x7fffffff;
    best_total_price = 0x7fffffff;

    for (int i = 0; i < train_count; i++) {
        if (!trains[i].released || trains[i].deleted) continue;

        Train& t1 = trains[i];
        int from_idx1 = get_station_idx(t1, from);
        if (from_idx1 < 0) continue;

        int start_min1 = time_to_minutes(t1.startTime);

        for (int j = 0; j < t1.stationNum; j++) {
            if (j <= from_idx1) continue;

            int arrive_day1, arrive_min1;
            add_minutes(date, start_min1, t1.travelTimes[from_idx1], arrive_day1, arrive_min1);
            for (int k = from_idx1; k < j - 1; k++) {
                add_minutes(arrive_day1, arrive_min1, t1.stopoverTimes[k] + t1.travelTimes[k+1], arrive_day1, arrive_min1);
            }

            int transfer_day = arrive_day1;
            int transfer_min = arrive_min1;

            for (int k = 0; k < train_count; k++) {
                if (i == k) continue;
                if (!trains[k].released || trains[k].deleted) continue;

                Train& t2 = trains[k];
                int to_idx2 = get_station_idx(t2, to);
                if (to_idx2 < 0) continue;

                for (int m = 0; m < t2.stationNum; m++) {
                    if (m >= to_idx2) continue;

                    int start_min2 = time_to_minutes(t2.startTime);

                    int depart_day2 = transfer_day;
                    int depart_min2 = transfer_min;

                    int start_day2 = depart_day2;
                    int start_min2_abs = start_min2;

                    int leave_day2, leave_min2;
                    add_minutes(start_day2, start_min2_abs, t2.travelTimes[m], leave_day2, leave_min2);

                    if (depart_day2 < leave_day2 || (depart_day2 == leave_day2 && depart_min2 < leave_min2)) {
                        int end_day2, end_min2;
                        add_minutes(leave_day2, leave_min2, 0, end_day2, end_min2);
                        for (int n = m; n < to_idx2 - 1; n++) {
                            add_minutes(end_day2, end_min2, t2.stopoverTimes[n] + t2.travelTimes[n+1], end_day2, end_min2);
                        }

                        int duration1 = (arrive_day1 - date) * 24 * 60 + (arrive_min1 - start_min1);
                        int duration2 = (end_day2 - start_day2) * 24 * 60 + (end_min2 - start_min2_abs);
                        int total_duration = duration1 + duration2;

                        int price1 = t1.prices[j] - t1.prices[from_idx1];
                        int price2 = t2.prices[to_idx2] - t2.prices[m];
                        int total_price = price1 + price2;

                        bool better = false;
                        if (str_equal(priority, "time")) {
                            if (total_duration < best_total_duration) better = true;
                            else if (total_duration == best_total_duration) {
                                if (duration1 < (best_transfer1.end_day - best_transfer1.start_day) * 24 * 60 + (best_transfer1.end_min - best_transfer1.start_min)) {
                                    better = true;
                                }
                            }
                        } else {
                            if (total_price < best_total_price) better = true;
                        }

                        if (better || !found_transfer) {
                            found_transfer = true;
                            best_total_duration = total_duration;
                            best_total_price = total_price;

                            best_transfer1.train_idx = i;
                            best_transfer1.from_idx = from_idx1;
                            best_transfer1.to_idx = j;
                            best_transfer1.start_day = date;
                            best_transfer1.start_min = start_min1;
                            best_transfer1.end_day = arrive_day1;
                            best_transfer1.end_min = arrive_min1;
                            best_transfer1.price = price1;
                            best_transfer1.duration = duration1;

                            best_transfer2.train_idx = k;
                            best_transfer2.from_idx = m;
                            best_transfer2.to_idx = to_idx2;
                            best_transfer2.start_day = start_day2;
                            best_transfer2.start_min = start_min2_abs;
                            best_transfer2.end_day = end_day2;
                            best_transfer2.end_min = end_min2;
                            best_transfer2.price = price2;
                            best_transfer2.duration = duration2;
                        }
                    }
                }
            }
        }
    }

    if (!found_transfer) {
        sprintf(output, "0\n");
        return;
    }

    char* out_ptr = output;
    Train& t1 = trains[best_transfer1.train_idx];
    Train& t2 = trains[best_transfer2.train_idx];

    char start_date1[20], end_date1[20];
    int_to_date(best_transfer1.start_day, start_date1);
    int_to_date(best_transfer1.end_day, end_date1);

    int min_seat1 = t1.seats[best_transfer1.start_day][best_transfer1.from_idx];
    for (int j = best_transfer1.from_idx; j < best_transfer1.to_idx; j++) {
        if (t1.seats[best_transfer1.start_day][j] < min_seat1) {
            min_seat1 = t1.seats[best_transfer1.start_day][j];
        }
    }

    out_ptr += sprintf(out_ptr, "%s %s %s %02d:%02d -> %s %s %02d:%02d %d %d\n",
                t1.trainID, t1.stations[best_transfer1.from_idx], start_date1,
                best_transfer1.start_min / 60, best_transfer1.start_min % 60,
                t1.stations[best_transfer1.to_idx], end_date1, best_transfer1.end_min / 60, best_transfer1.end_min % 60,
                best_transfer1.price, min_seat1);

    char start_date2[20], end_date2[20];
    int_to_date(best_transfer2.start_day, start_date2);
    int_to_date(best_transfer2.end_day, end_date2);

    int min_seat2 = t2.seats[best_transfer2.start_day][best_transfer2.from_idx];
    for (int j = best_transfer2.from_idx; j < best_transfer2.to_idx; j++) {
        if (t2.seats[best_transfer2.start_day][j] < min_seat2) {
            min_seat2 = t2.seats[best_transfer2.start_day][j];
        }
    }

    out_ptr += sprintf(out_ptr, "%s %s %s %02d:%02d -> %s %s %02d:%02d %d %d\n",
                t2.trainID, t2.stations[best_transfer2.from_idx], start_date2,
                best_transfer2.start_min / 60, best_transfer2.start_min % 60,
                t2.stations[best_transfer2.to_idx], end_date2, best_transfer2.end_min / 60, best_transfer2.end_min % 60,
                best_transfer2.price, min_seat2);
}

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

int buy_ticket(const char* username, const char* trainID, int date, int num,
                const char* from, const char* to, bool queue_flag, char* output) {
    int user_idx = user_map.get(username);
    if (user_idx < 0) return -1;
    if (!users[user_idx].online) return -1;

    int train_idx = train_map.get(trainID);
    if (train_idx < 0) return -1;
    if (!trains[train_idx].released || trains[train_idx].deleted) return -1;

    Train& t = trains[train_idx];
    int from_idx = get_station_idx(t, from);
    int to_idx = get_station_idx(t, to);

    if (from_idx < 0 || to_idx < 0 || from_idx >= to_idx) return -1;
    if (num <= 0 || num > t.seatNum) return -1;

    int min_seat = t.seats[date][from_idx];
    for (int i = from_idx; i < to_idx; i++) {
        if (t.seats[date][i] < min_seat) {
            min_seat = t.seats[date][i];
        }
    }

    int start_min = time_to_minutes(t.startTime);
    int end_day, end_min;
    add_minutes(date, start_min, t.travelTimes[from_idx], end_day, end_min);
    for (int i = from_idx; i < to_idx - 1; i++) {
        add_minutes(end_day, end_min, t.stopoverTimes[i] + t.travelTimes[i+1], end_day, end_min);
    }

    int price = (t.prices[to_idx] - t.prices[from_idx]) * num;

    if (min_seat >= num) {
        for (int i = from_idx; i < to_idx; i++) {
            t.seats[date][i] -= num;
        }

        int order_idx = order_count++;
        Order& o = orders[order_idx];
        str_copy(o.username, username);
        str_copy(o.trainID, trainID);
        str_copy(o.from, from);
        str_copy(o.to, to);
        o.date = date;
        o.start_min = start_min;
        o.end_min = end_min;
        o.end_day = end_day;
        o.price = t.prices[to_idx] - t.prices[from_idx];
        o.num = num;
        o.train_idx = train_idx;
        o.from_idx = from_idx;
        o.to_idx = to_idx;
        o.status = 0;
        o.timestamp = order_count;
        o.queue = false;

        user_orders[user_idx][user_order_count[user_idx]++] = &orders[order_idx];

        sprintf(output, "%d", price);
        return 0;
    } else if (queue_flag) {
        int order_idx = order_count++;
        Order& o = orders[order_idx];
        str_copy(o.username, username);
        str_copy(o.trainID, trainID);
        str_copy(o.from, from);
        str_copy(o.to, to);
        o.date = date;
        o.start_min = start_min;
        o.end_min = end_min;
        o.end_day = end_day;
        o.price = t.prices[to_idx] - t.prices[from_idx];
        o.num = num;
        o.train_idx = train_idx;
        o.from_idx = from_idx;
        o.to_idx = to_idx;
        o.status = 1;
        o.timestamp = order_count;
        o.queue = true;

        user_orders[user_idx][user_order_count[user_idx]++] = &orders[order_idx];

        sprintf(output, "queue");
        return 0;
    }

    return -1;
}

int query_order(const char* username, char* output) {
    int user_idx = user_map.get(username);
    if (user_idx < 0) return -1;
    if (!users[user_idx].online) return -1;

    char* out_ptr = output;
    out_ptr += sprintf(out_ptr, "%d\n", user_order_count[user_idx]);

    for (int i = user_order_count[user_idx] - 1; i >= 0; i--) {
        Order& o = *user_orders[user_idx][i];
        Train& t = trains[o.train_idx];

        const char* status_str;
        if (o.status == 0) status_str = "success";
        else if (o.status == 1) status_str = "pending";
        else status_str = "refunded";

        char start_date[20], end_date[20];
        int_to_date(o.date, start_date);
        int_to_date(o.end_day, end_date);

        out_ptr += sprintf(out_ptr, "[%s] %s %s %s %02d:%02d -> %s %s %02d:%02d %d %d\n",
                    status_str, t.trainID, t.stations[o.from_idx], start_date,
                    o.start_min / 60, o.start_min % 60,
                    t.stations[o.to_idx], end_date, o.end_min / 60, o.end_min % 60,
                    o.price, o.num);
    }
    return 0;
}

int refund_ticket(const char* username, int n) {
    int user_idx = user_map.get(username);
    if (user_idx < 0) return -1;
    if (!users[user_idx].online) return -1;

    if (n < 1 || n > user_order_count[user_idx]) return -1;

    int idx = user_order_count[user_idx] - n;
    Order& o = *user_orders[user_idx][idx];

    if (o.status == 2) return -1;

    if (o.status == 0) {
        Train& t = trains[o.train_idx];
        for (int i = o.from_idx; i < o.to_idx; i++) {
            t.seats[o.date][i] += o.num;
        }
    }

    o.status = 2;
    return 0;
}

void clean() {
    user_count = 0;
    train_count = 0;
    order_count = 0;
    user_map.init();
    online_users.init();
    train_map.init();
    for (int i = 0; i < MAX_USERS; i++) {
        user_order_count[i] = 0;
    }
}

int main() {
    string_pool = (StringNode*)malloc(sizeof(StringNode) * HASH_SIZE * 10);
    string_pool_ptr = 0;

    user_map.init();
    online_users.init();
    train_map.init();

    for (int i = 0; i < MAX_USERS; i++) {
        user_order_count[i] = 0;
    }

    char line[2000];
    char command[50];

    while (fgets(line, sizeof(line), stdin)) {
        if (line[0] == '\n') continue;

        char* cmd_ptr = line;
        while (*cmd_ptr && *cmd_ptr != '\n') cmd_ptr++;
        *cmd_ptr = '\0';

        cmd_ptr = line;
        while (*cmd_ptr == ' ') cmd_ptr++;

        sscanf(cmd_ptr, "%s", command);
        cmd_ptr += strlen(command);
        while (*cmd_ptr == ' ') cmd_ptr++;

        char output[50000];

        if (str_equal(command, "add_user")) {
            char cur[30] = "", u[30], pw[40], nm[20], mail[40], g_str[10];
            int privilege = -1;

            while (*cmd_ptr) {
                char key[5], val[100];
                sscanf(cmd_ptr, "-%s %s", key, val);
                cmd_ptr += strlen(key) + strlen(val) + 3;
                while (*cmd_ptr == ' ') cmd_ptr++;

                if (key[0] == 'c') str_copy(cur, val);
                else if (key[0] == 'u') str_copy(u, val);
                else if (key[0] == 'p') str_copy(pw, val);
                else if (key[0] == 'n') str_copy(nm, val);
                else if (key[0] == 'm') str_copy(mail, val);
                else if (key[0] == 'g') privilege = atoi(val);
            }

            int result = add_user(cur, u, pw, nm, mail, privilege);
            printf("%d\n", result);

        } else if (str_equal(command, "login")) {
            char u[30], pw[40];
            sscanf(cmd_ptr, "-u %s -p %s", u, pw);
            printf("%d\n", login(u, pw));

        } else if (str_equal(command, "logout")) {
            char u[30];
            sscanf(cmd_ptr, "-u %s", u);
            printf("%d\n", logout(u));

        } else if (str_equal(command, "query_profile")) {
            char c[30], u[30];
            sscanf(cmd_ptr, "-c %s -u %s", c, u);
            if (query_profile(c, u, output) == 0) {
                printf("%s\n", output);
            } else {
                printf("-1\n");
            }

        } else if (str_equal(command, "modify_profile")) {
            char c[30], u[30], pw[40] = "", nm[20] = "", mail[40] = "", g_str[10] = "";
            int privilege = -1;

            while (*cmd_ptr) {
                char key[5], val[100];
                sscanf(cmd_ptr, "-%s %s", key, val);
                cmd_ptr += strlen(key) + strlen(val) + 3;
                while (*cmd_ptr == ' ') cmd_ptr++;

                if (key[0] == 'c') str_copy(c, val);
                else if (key[0] == 'u') str_copy(u, val);
                else if (key[0] == 'p') str_copy(pw, val);
                else if (key[0] == 'n') str_copy(nm, val);
                else if (key[0] == 'm') str_copy(mail, val);
                else if (key[0] == 'g') privilege = atoi(val);
            }

            if (modify_profile(c, u, pw, nm, mail, privilege, output) == 0) {
                printf("%s\n", output);
            } else {
                printf("-1\n");
            }

        } else if (str_equal(command, "add_train")) {
            char i[30], s[500], p[500], x[10], t[500], o[500], d[20], y[2];
            int n, m;

            while (*cmd_ptr) {
                char key[5], val[500];
                sscanf(cmd_ptr, "-%s %s", key, val);
                cmd_ptr += strlen(key) + strlen(val) + 3;
                while (*cmd_ptr == ' ') cmd_ptr++;

                if (key[0] == 'i') str_copy(i, val);
                else if (key[0] == 'n') n = atoi(val);
                else if (key[0] == 'm') m = atoi(val);
                else if (key[0] == 's') str_copy(s, val);
                else if (key[0] == 'p') str_copy(p, val);
                else if (key[0] == 'x') str_copy(x, val);
                else if (key[0] == 't') str_copy(t, val);
                else if (key[0] == 'o') str_copy(o, val);
                else if (key[0] == 'd') str_copy(d, val);
                else if (key[0] == 'y') y[0] = val[0];
            }

            printf("%d\n", add_train(i, n, m, s, p, x, t, o, d, y[0]));

        } else if (str_equal(command, "release_train")) {
            char i[30];
            sscanf(cmd_ptr, "-i %s", i);
            printf("%d\n", release_train(i));

        } else if (str_equal(command, "delete_train")) {
            char i[30];
            sscanf(cmd_ptr, "-i %s", i);
            printf("%d\n", delete_train(i));

        } else if (str_equal(command, "query_train")) {
            char i[30], d[10];
            sscanf(cmd_ptr, "-i %s -d %s", i, d);
            int date = date_to_int(d);
            if (query_train(i, date, output) == 0) {
                printf("%s", output);
            } else {
                printf("-1\n");
            }

        } else if (str_equal(command, "query_ticket")) {
            char s[20], t[20], d[10], priority[10] = "time";
            sscanf(cmd_ptr, "-s %s -t %s -d %s", s, t, d);

            char* q = strstr(cmd_ptr, "-p");
            if (q) {
                sscanf(q, "-p %s", priority);
            }

            int date = date_to_int(d);
            query_ticket(s, t, date, priority, output);
            printf("%s", output);

        } else if (str_equal(command, "query_transfer")) {
            char s[20], t[20], d[10], priority[10] = "time";
            sscanf(cmd_ptr, "-s %s -t %s -d %s", s, t, d);

            char* q = strstr(cmd_ptr, "-p");
            if (q) {
                sscanf(q, "-p %s", priority);
            }

            int date = date_to_int(d);
            query_transfer(s, t, date, priority, output);
            printf("%s", output);

        } else if (str_equal(command, "buy_ticket")) {
            char u[30], i[30], d[10], f[20], t[20], q_str[10] = "false";
            int n;

            while (*cmd_ptr) {
                char key[5], val[100];
                sscanf(cmd_ptr, "-%s %s", key, val);
                cmd_ptr += strlen(key) + strlen(val) + 3;
                while (*cmd_ptr == ' ') cmd_ptr++;

                if (key[0] == 'u') str_copy(u, val);
                else if (key[0] == 'i') str_copy(i, val);
                else if (key[0] == 'd') str_copy(d, val);
                else if (key[0] == 'n') n = atoi(val);
                else if (key[0] == 'f') str_copy(f, val);
                else if (key[0] == 't') str_copy(t, val);
                else if (key[0] == 'q') str_copy(q_str, val);
            }

            bool queue_flag = str_equal(q_str, "true");
            int date = date_to_int(d);

            if (buy_ticket(u, i, date, n, f, t, queue_flag, output) == 0) {
                printf("%s\n", output);
            } else {
                printf("-1\n");
            }

        } else if (str_equal(command, "query_order")) {
            char u[30];
            sscanf(cmd_ptr, "-u %s", u);
            if (query_order(u, output) == 0) {
                printf("%s", output);
            } else {
                printf("-1\n");
            }

        } else if (str_equal(command, "refund_ticket")) {
            char u[30], n_str[10] = "1";
            sscanf(cmd_ptr, "-u %s", u);

            char* q = strstr(cmd_ptr, "-n");
            if (q) {
                sscanf(q, "-n %s", n_str);
            }

            int n = atoi(n_str);
            printf("%d\n", refund_ticket(u, n));

        } else if (str_equal(command, "clean")) {
            clean();
            printf("0\n");

        } else if (str_equal(command, "exit")) {
            printf("bye\n");
            break;
        }
    }

    return 0;
}
