//реализация LR(0) разбора с древовидным стеком 
//при возникновении конфликтов для каждой ветви отвечающей за свою операцию, сдвиг или свертка
//создается отдельный ветвь стек  - копия текущего стека 
//дальше каждая отдельная ветвь обрабатывается отдельно 

//в моей реализации все нетерминалы - одна большая буква латинского алфавита
//нетерминалы - все остальные символы

//важно : S - стартовый нетерминал
// Z - так называемый предстартовый нетерминал ! Им пополняется грамматика перед тем как конструируется 
// LR(0) - автомат , поэтому если использовать Z в исходной грамматике
// разбор будет осуществляться некорректно

#include <iostream>
#include <vector>
#include <set>
#include <unordered_map>
#include <string>
#include <stack>
#include <algorithm>

using namespace std;

void print_states(vector<set<string>>& states){
    for (int i = 0; i < states.size(); i++){
        cout << "state " << i << endl;
        for (auto elem : states[i]){
            cout << "  " << elem << endl;
        }
    }
}

string put_point_before_right_part(string rule){
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

void closure(vector<string>& grammar, char neter, vector<set<string>>& states, int state_num){
    for (auto rule : grammar){
        if (rule[0] == neter){
            states[state_num].insert(put_point_before_right_part(rule));
        }
    }
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
    if (point_pos + 1 < item.size()){
        res.push_back(item[point_pos + 1]);
    }
    res.push_back('.');
    for (int i = point_pos + 2; i < item.length(); i++){
        res.push_back(item[i]);
    }
    return res;
}

bool is_letter_neter(char letter){
    if (letter >= 'A' && letter <= 'Z'){
        return true;
    }
    return false;
}

bool is_point_at_the_end_of_item(string item){
    return (item[item.length() - 1] == '.');
}


char get_letter_after_point(string item){
    int point_pos = -1;
    for (int i = 0; i < item.size(); i++){
        if (item[i] == '.'){
            point_pos = i;
        }
    }
    return item[point_pos + 1];
}

set<string> go_to(vector<set<string>>& states, char letter, int state_num){
    set<string>res;
    for (auto item : states[state_num]){
        if (get_letter_after_point(item) == letter){
            res.insert(move_point_to_the_right(item));
        }
    }
    return res;
}

void fill_state_by_closure(vector<string>& grammar, vector<set<string>>& states, int state_num){
    unordered_map<string, bool> proceeded_closure;
    bool flag = true;
    while (flag){
        flag = false;
        for (auto item : states[state_num]){
            if (!is_point_at_the_end_of_item(item) && !proceeded_closure[item] 
            && is_letter_neter(get_letter_after_point(item))){
                proceeded_closure[item] = true;
                closure(grammar, get_letter_after_point(item), states, state_num);
                flag = true;
            }
        }
    }
}

void fill_states(vector<set<string>>& states, vector<string>& grammar, vector<vector<pair<char, int>>>& transitions){
    int cur_state = 0;
    states[cur_state].insert(put_point_before_right_part("Z:S"));
    fill_state_by_closure(grammar, states, cur_state);
    while (cur_state < states.size()){
        transitions.push_back({});
        //делаем go_to по всем возможным символам после точки 
        unordered_map<char, bool>proceeded_letters;
        vector<set<string>> new_states;
        vector<char>new_states_chars;
        for (auto item : states[cur_state]){
            if (!is_point_at_the_end_of_item(item) && !proceeded_letters[get_letter_after_point(item)]){
                proceeded_letters[get_letter_after_point(item)] = true;
                new_states.push_back(go_to(states, get_letter_after_point(item), cur_state));
                new_states_chars.push_back(get_letter_after_point(item));
            }
        }
        //ко всем созданным с помощью go to состояниям применяем closure
        for (int i = 0; i < new_states.size(); i++){
            fill_state_by_closure(grammar, new_states, i);
        }
        for (int i = 0; i < new_states.size(); i++){
            bool flag = true;
            for (int j = 0; j < states.size(); j++){
                if (states[j] == new_states[i]){
                    //если созданное состояние совпало с уже существующим, помечаю  переход 
                    //по текущему состоянию и символу в старое состояние, новое брать не будем
                    transitions[cur_state].push_back({new_states_chars[i], j});
                    flag = false;
                }
            }
            if (flag){
                states.push_back(new_states[i]);
                // помечаем переходы по текущему состоянию и символу в те состояния, 
                // которые не совпали с уже существующими и которые мы поэтому берем 
                transitions[cur_state].push_back({new_states_chars[i], states.size() - 1});
            }
        }
        cur_state++;
    }
}

void add_prestart_symbol_in_grammar(vector<string>& grammar){
    grammar.push_back("Z:S");
}

void get_grammar_terms_and_nonterms(vector<string>& grammar, vector<char>& terms, vector<char>& nonterms){
    set<char> terms_set;
    set<char> nonterms_set;
    for (int i = 0; i < grammar.size(); i++){
        for (int j = 0; j < grammar[i].length(); j++){
            if (is_letter_neter(grammar[i][j])){
                nonterms_set.insert(grammar[i][j]);
            }else if (j != 1){ 
                terms_set.insert(grammar[i][j]);
            }
        }
    }
    for (auto elem : nonterms_set){
        nonterms.push_back(elem);
    }
    for (auto elem : terms_set){
        terms.push_back(elem);
    }
}

string remove_point_from_item(string item){
    string res = "";
    for (auto letter : item){
        if (letter != '.'){
            res.push_back(letter);
        }
    }
    return res;
}

int get_number_of_reduce(vector<string>& grammar, string item){
    for (int i = 0; i < grammar.size(); i++){
        if (grammar[i] == remove_point_from_item(item)){
            return i;
        }
    }
    return -1;
}

vector<unordered_map<char, string>> create_control_table (vector<string>& grammar, vector<set<string>>& states,
 vector<vector<pair<char, int>>>& transitions, vector<char>& terms, vector<char>& nonterms){
        vector<unordered_map<char, string>> control_table (states.size());
        for (int i = 0; i < transitions.size(); i++){
            for (auto elem : transitions[i]){
                if (is_letter_neter(elem.first)){
                    control_table[i][elem.first] = to_string(elem.second);
                }else{
                    control_table[i][elem.first] = "s" + to_string(elem.second);
                }
            }
        }
        for (int i = 0; i < states.size(); i++){
            for (auto item : states[i]){
                if (is_point_at_the_end_of_item(item)){
                    if (item[0] == 'Z'){
                        control_table[i]['$'] += "acc";
                    }else{
                        for (auto term : terms){
                            control_table[i][term] += "r" + to_string(get_number_of_reduce(grammar, item));
                        }
                    }
                }
            }
        }
        return control_table;
}

bool check_control_table_on_conflicts(
    vector<unordered_map<char, string>>& control_table, vector<char>& terms){
    for (int i = 0; i < control_table.size(); i++){
        for (auto elem : terms){
            string actions = control_table[i][elem];
            int num_reduces = 0;
            int num_shifts = 0;
            for (int i = 0; i < actions.length(); i+=2){
                if (actions[i] == 's'){
                    num_shifts++;
                }
                if (actions[i] == 'r'){
                    num_reduces++;
                }
            }
            bool flag = true;
            if (num_shifts != 0 && num_reduces != 0){
                cout << " есть конфликт сдвиг свертка " << endl;
                flag = false;
            }
            if (num_reduces > 1){
                cout << " есть конфликт свертка свертка " << endl;
                flag = false;
            }
            if (!flag){
                return false;
            }
        }
    }
    return true;
}

void print_control_table(
    vector<unordered_map<char, string>>& control_table, vector<char>& terms, vector<char>& nonterms){
        cout << "   ";
        for (auto elem : nonterms){
            cout << elem << "  ";
        }
        for (auto elem : terms){
            cout << elem << "  ";
        }
        cout << endl;
        for (int i = 0 ; i < control_table.size(); i++){
            cout << i << "  ";
            for (auto elem : nonterms){
                if (control_table[i][elem].length() != 0){
                    cout << control_table[i][elem] << "  ";
                }else{
                    cout << "-  ";
                }
            }
            for (auto elem : terms){
                if (control_table[i][elem].length() != 0){
                    cout << control_table[i][elem] << " ";
                }else{
                    cout << "-  ";
                }
            }
            cout << endl;
        }
}

bool is_word_correct(string action, char cur_token){
    if (action == "acc" && cur_token == '$'){
        return true;
    }
    return false;
}

bool lr0_parsing (vector<string>& grammar, vector<unordered_map<char, string>>& control_table,
 stack<char> word_stack, vector<char>& terms){
    stack<string> parse_stack;
    parse_stack.push(to_string(0));
    int count = 0;
    while (true){
        string top_elem = parse_stack.top();
        char cur_token = word_stack.top();
        int cur_state = top_elem[top_elem.length() - 1] - '0';
        string action = control_table[cur_state][cur_token];
        if (find(terms.begin(), terms.end(), cur_token) == terms.end()){
            cout << "недопустимый токен " << cur_token << endl;
            return false;
        }
        if (action.size() == 0 || action == "acc"){
            return is_word_correct(action , cur_token);
        }
        string new_top_elem;
        if (action[0] == 's'){
            new_top_elem.push_back(cur_token);
            new_top_elem.push_back(action[1]);
            word_stack.pop();
        }else if (action[0] == 'r'){
            string needed_rule = grammar[action[1] - '0'];
            int num_elems_to_pop = needed_rule.length() - 2;
            for (int i = 0; i < num_elems_to_pop; i++){
                parse_stack.pop();
            }
            top_elem = parse_stack.top();
            cur_state = top_elem[top_elem.length() - 1] - '0';
            action = control_table[cur_state][needed_rule[0]];
            if (action.size() == 0 || action == "acc"){
                return is_word_correct(action, cur_token);
            }
            new_top_elem.push_back(needed_rule[0]);
            new_top_elem.push_back(action[0]);
        }
        parse_stack.push(new_top_elem);
        count++;
    }
}

int main(){
    vector<set<string>> states = {{}};
    vector<string>grammar;
    cout << "введите количество правил в грамматике" << endl;
    int n;
    cin >> n;
    cout << "введите " << n << " правил" << endl;
    string rule;
    for (int i = 0 ; i < n; i++){
        cin >> rule;
        grammar.push_back(rule);
    }
    add_prestart_symbol_in_grammar(grammar);
    cout << "введите слово " << endl;
    string word;
    cin >> word;
    stack<char>word_stack;
    word_stack.push('$');
    for (int i = word.size() - 1; i >= 0; i--){
        word_stack.push(word[i]);
    }
    // переходы, которые помечаются сразу при создании и заполнении новых состояний
    // по ним будем строить полноценную управляющую таблицу
    vector<vector<pair<char, int>>> transitions;
    fill_states(states, grammar, transitions);
    cout << "\nstates of automat : " << endl;
    print_states(states);
    vector<char>terms;
    vector<char>nonterms;
    get_grammar_terms_and_nonterms(grammar, terms, nonterms);
    terms.push_back('$');
    vector<unordered_map<char, string>> control_table = create_control_table(grammar, 
    states, transitions, terms, nonterms);
    cout << "\ncontrol table : " << endl;
    print_control_table(control_table, terms, nonterms);
    if (check_control_table_on_conflicts(control_table, terms)){
        cout << "конфликтов нет " << endl;
        if (lr0_parsing(grammar, control_table, word_stack, terms)){
            cout << "слово успешно разобрано ! " << endl;
        }else{
            cout << "слово не разбирается с помощью данной грамматики " << endl;
        }
    }else{
        cout << "слово не разбирается с помощью данной грамматики " << endl;
    }
    return 0;
}