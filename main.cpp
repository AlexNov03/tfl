#include <iostream>
#include <unordered_map>
#include <vector>
#include <set>
#include<string.h>

using namespace std;

string put_point_to_the_begin_of_rule(string rule){
    string res = "";
    int colon_pos = 0;
    for (int i = 0; i < rule.length() && rule[i] != ':'; i++){
        res.push_back(rule[i]);
        colon_pos ++;
    }
    res += ":.";
    for (int i = colon_pos + 1; i < rule.length(); i++){
        res.push_back(rule[i]);
    }
    
    return res;
}

string move_point_to_the_right(string item){
    int point_pos = -1;
    for (int i = 0; i < item.length(); i++){
        if (item[i] == '.'){
            point_pos = i;
        }
    }
    string res = "";
    for (int i = 0; i < point_pos; i++){
        res.push_back(item[i]);
    }
    res.push_back(item[point_pos + 1]);
    res.push_back('.');
    for (int i = point_pos + 2; i < item.length(); i++){
        res.push_back(item[i]);
    }
    return res;
}

char is_point_before_term(pair<string, int> item){
    char res;
    int point_pos = -1;
    for (int i = 0; i < item.first.length(); i++){
        if (item.first[i] == '.'){
            point_pos = i;
        }
    }
    if (point_pos < item.first.length() && (item.first[point_pos + 1] < 'A' ||
    item.first[point_pos + 1] > 'Z')){
        res = item.first[point_pos + 1];
    }
    return res;
}

char is_point_before_neter(pair<string, int> item){
    char res;
    int point_pos = -1;
    for (int i = 0; i < item.first.length(); i++){
        if (item.first[i] == '.'){
            point_pos = i;
        }
    }
    if (point_pos < item.first.length() && item.first[point_pos + 1] >= 'A' &&
    item.first[point_pos + 1] <= 'Z'){
        res = item.first[point_pos + 1];
    }
    return res;
}

char  is_neter_finished(pair<string, int> item){
    char res;
    if (item.first[item.first.length() - 1] == '.'){
        res = item.first[0];
    }
    return res;
}

void insert_items_where_neter_finished(
    vector<set<pair<string, int>>>& itemset, char finished_neter, int k, int item_num){
    for (auto elem : itemset[k]){
        char neter = is_point_before_neter(elem);
        if (neter == finished_neter){
            elem.first = move_point_to_the_right(elem.first);
            itemset[item_num].insert(elem);
        }
    }
}

void insert_items_where_point_before_term(
    vector<set<pair<string, int>>>& itemset, char term, int k, int item_num){
    for (auto elem : itemset[k]){
        char term1 = is_point_before_term(elem);
        if (term1 == term){
            elem.first = move_point_to_the_right(elem.first);
            itemset[item_num].insert(elem);
        }
    }
}



void insert_items_from_grammar(
    vector<set<pair<string, int>>>& itemset, vector<string>grammar, char neter, int item_num){
    for (auto elem : grammar){
        if (elem[0] == neter){
            itemset[item_num].insert({put_point_to_the_begin_of_rule(elem), item_num});
        }
    }
}


vector<set<pair<string, int>>> early_parser (vector<string>grammar, string w){
    vector<set<pair<string, int>>> itemset (w.size() + 1);
    for (int i = 0; i < w.size() + 1; i++){
        unordered_map<string, bool>proceeded;
        if (i == 0){
            insert_items_from_grammar(itemset, grammar, 'S', 0);
        }else{
            insert_items_where_point_before_term(itemset, w[i - 1], i - 1, i);
        }
        bool flag = true;
        while (flag){
            flag = false;
            for (auto elem : itemset[i]){
                string s = elem.first;
                s.push_back('0' + elem.second);
                if (!proceeded[s]){
                    proceeded[s] = true;
                    char neter = is_point_before_neter(elem);
                    if (neter){
                        insert_items_from_grammar(itemset, grammar, neter, i);
                        flag = true;
                    }
                    neter = is_neter_finished(elem);
                    if (neter){
                        insert_items_where_neter_finished(itemset, neter, elem.second, i);
                        flag = true;
                    }
                }
                
            }
        }
    }
    return itemset;
}

void print_itemset(vector<set<pair<string, int>>>& itemset){
    for (int i = 0; i < itemset.size(); i++){
        cout << "item " << i << endl;
        for (auto elem : itemset[i]){
            cout << elem.first << " " << elem.second << endl;
        }
    }
}

bool does_word_belong(vector<set<pair<string, int>>>& itemset){
    for (auto elem : itemset[itemset.size() - 1]){
        if (elem.first[0] == 'S' && elem.first[elem.first.length() - 1] == '.' && elem.second == 0){
            return true;
        }
    }
    return false;
}

int main(){
    vector<string> grammar;
    cout << endl;
    cout << "ввод грамматики осуществляется следующим образом" << endl;
    cout << "2" << endl;
    cout << "S:T" << endl;
    cout << "T:abc" << endl;
    cout << "где 2 - количество правил в грамматике, ниже приведены сами правила" << endl;
    cout << "нетерминалы - большие буквы, S - стартовый нетерминал" << endl;
    cout << "терминалы - все остальные символы" << endl;
    cout << "введите количество правил" << endl;
    int n;
    cin >> n;
    cout << "введите " << n << " правил" << endl;
    string rule;
    for (int i = 0; i < n; i++){
        cin >> rule;
        grammar.push_back(rule);
    }
    cout << "введите слово " << endl;
    string w;
    cin >> w;
    vector<set<pair<string, int>>> itemset = early_parser(grammar, w);
    print_itemset(itemset);
    if (does_word_belong(itemset)){
        cout << "слово " << w << " выводится данной грамматикой " << endl;
    }else{
        cout << "слово " << w << " не выводится данной грамматикой " << endl;
    }
    return 0;
}