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

struct Conflict_States{
    const string no_conflicts {"no_conflicts"};
    const string shift_reduce {"shift_reduce"};
    const string reduce_reduce {"reduce_reduce"};
};

string check_state_on_conflicts(unordered_map<char, string>& state, char cur_token){
    string actions = state[cur_token];
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
    Conflict_States conflict_states;
    if (num_shifts != 0 && num_reduces != 0){
        return conflict_states.shift_reduce;
    }
    if (num_reduces > 1){
        return conflict_states.reduce_reduce;
    }
    return conflict_states.no_conflicts;
}

bool is_word_correct(string action, char cur_token){
    if (action == "acc" && cur_token == '$'){
        return true;
    }
    return false;
}

struct Node{
    Node* parent;
    string val;
    Node():parent(nullptr), val(""){}
    Node(string _val):parent(nullptr), val(_val){}
    Node(Node* parent, string _val):parent(parent), val(_val){}
};

void draw_branch(Node* top){
    if (top != nullptr){
        cout << top -> val << " -> ";
        draw_branch(top -> parent);
    }
    cout << endl;
}

void print_completion_message(bool is_word_correct, Node* top){
    if (is_word_correct){
        cout << " разбор прошел удачно ! " << endl;
        draw_branch(top);
    }else{
        cout << " ветка потерпела неудачу ! " << endl;
        draw_branch(top);
    }
}

Node* push_val(Node* top, string val){
    Node* new_top = new Node(top, val);
    return new_top;
}

Node* pop_val(Node* top){
    Node* new_top = top -> parent;
    free(top);
    return new_top;
}

Node* do_shift(Node* top, string action, char cur_token){
    string new_val;
    new_val.push_back(cur_token);
    new_val.push_back(action[1]);
    top = push_val(top, new_val);
    return top;
}

pair<bool, Node*> do_reduce(Node* top, vector<string>& grammar, string action, char cur_token,
vector<unordered_map<char, string>>& control_table){
    string needed_rule = grammar[action[1] - '0'];
    int num_elems_to_pop = needed_rule.length() - 2;
    for (int i = 0; i < num_elems_to_pop; i++){
        top = pop_val(top);
    }
    string top_elem = top->val;
    int cur_state = top_elem[top_elem.length() - 1] - '0';
    action = control_table[cur_state][needed_rule[0]];
    if (action.size() == 0 || action == "acc"){
        if (is_word_correct(action, cur_token)){
            return {true, nullptr};
        }else{
            return {false, nullptr};
        }
    }
    string new_val;
    new_val.push_back(needed_rule[0]);
    new_val.push_back(action[0]);
    top = push_val(top, new_val);
    return {false, top};
}

pair<bool, Node*> make_new_reduce_branch(Node* top, vector<string>& grammar, string action, char cur_token,
vector<unordered_map<char, string>>& control_table){
    string needed_rule = grammar[action[1] - '0'];
    int num_elems_to_pop = needed_rule.length() - 2;
    Node* top_copy = top;
    for (int i = 0; i < num_elems_to_pop; i++){
        top_copy = top_copy -> parent;
    }
    string top_elem = top_copy->val;
    int cur_state = top_elem[top_elem.length() - 1] - '0';
    action = control_table[cur_state][needed_rule[0]];
    if (action.size() == 0 || action == "acc"){
        if (is_word_correct(action, cur_token)){
            return {true, nullptr};
        }else{
            return {false, nullptr};
        }
    }
    string new_val;
    new_val.push_back(needed_rule[0]);
    new_val.push_back(action[0]);
    top_copy = push_val(top_copy, new_val);
    return {false, top_copy};
}

bool tree_lr0_parsing (Node* top, int position, vector<string>& grammar, vector<unordered_map<char, string>>& control_table,
 stack<char> word_stack, vector<char>& terms){
    while (true){
        string top_elem = top->val;
        char cur_token = word_stack.top();
        if (find(terms.begin(), terms.end(), cur_token) == terms.end()){
            cout << "недопустимый токен " << cur_token << " в позиции слова " << position << endl;
            cout << " это первая ошибка в разборе " << endl;
            return false;
        }
        int cur_state = top_elem[top_elem.length() - 1] - '0';
        Conflict_States conflict_states;
        if (check_state_on_conflicts(control_table[cur_state], cur_token) == conflict_states.no_conflicts){
            string action = control_table[cur_state][cur_token];
            if (action.size() == 0 || action == "acc"){
                if (is_word_correct(action, cur_token)){
                    return true;
                }else{
                    return false;
                }
            }else if (action[0] == 's'){
                top = do_shift(top, action, cur_token);
                word_stack.pop();
                position++;
            }else if (action[0] == 'r'){
                pair<bool, Node*> res =  do_reduce(top, grammar, action, cur_token, control_table);
                if (res.second == nullptr){
                    if (res.first == true){
                        return true;
                    }else{
                        return false;
                    }
                }else{
                    top = res.second;
                }
            }
        }else if (check_state_on_conflicts(control_table[cur_state], cur_token) == conflict_states.shift_reduce){
            string action = control_table[cur_state][cur_token];
            Node* new_branch;
            for (int i = 0; i < action.size(); i+= 2){
                if (action[i] == 'r'){
                    pair<bool, Node*> res =  make_new_reduce_branch(top, grammar, action.substr(i, 2), cur_token, control_table);
                    Node* new_branch;
                    if (res.second == nullptr){
                        if (res.first == true){
                            return true;
                        }
                    }else{
                        new_branch = res.second;
                    }
                    if (tree_lr0_parsing(new_branch, position, grammar, control_table, word_stack, terms)){
                        return true;
                    };
                }
            }
            for (int i = 0; i < action.size(); i += 2){
                if (action[i] == 's'){
                    top = do_shift(top, action.substr(i, 2), cur_token);
                    word_stack.pop();
                    position++;
                }
            }
        }else if (check_state_on_conflicts(control_table[cur_state], cur_token) == conflict_states.reduce_reduce){
            string action = control_table[cur_state][cur_token];
            for (int i = 0; i < action.size(); i+= 2){
                if (action[i] == 'r'){
                    pair<bool, Node*> res = make_new_reduce_branch(top, grammar, action.substr(i, 2), cur_token, control_table);
                    Node* new_branch;
                    if (res.second == nullptr){
                        if (res.first == true){
                            return true;
                        }
                    }else{
                        new_branch = res.second;
                    }
                    if (tree_lr0_parsing(new_branch, position, grammar, control_table, word_stack, terms)){
                        return true;
                    };
                }
            }
        }
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
    Node* top = new Node("0");
    if (tree_lr0_parsing(top, 0, grammar, control_table, word_stack, terms)){
        cout << " разбор завершен успешно ! " << endl;
    }else{
        cout << " разбор неуспешен, ни одна из ветвей не пришла к концу !" << endl;
    }
    return 0;
}