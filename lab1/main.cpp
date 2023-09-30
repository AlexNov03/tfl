#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <fstream>
#include <string>

using namespace std;

vector<vector<string>> make_vector_from_elem(string elem){
    vector<vector<string>> res;
    vector<string> str1 = {elem + "0"};
    vector<string> str2 = {elem + "1"};
    res.push_back(str1);
    res.push_back(str2);
    return res;
}

vector<vector<string>> make_matrix_from_elem(string elem){
    vector<vector<string>> res;
    vector<string> str1 = {elem + "00", elem + "01"};
    vector<string> str2 = {elem + "10", elem + "11"};
    res.push_back(str1);
    res.push_back(str2);
    return res;
}

vector<vector<string>> mult_matrix_by_matrix(vector<vector<string>> m1, vector<vector<string>> m2){
    vector<vector<string>> res = {{"", ""}, {"", ""}};
    res[0][0] = "(arctic_max (arctic_sum " + m1[0][0] + " " +  m2[0][0] + ") (arctic_sum " + m1[0][1] + " " +  m2[1][0] + "))";
    res[0][1] = "(arctic_max (arctic_sum " + m1[0][0] + " " +  m2[0][1] + ") (arctic_sum " + m1[0][1] + " " +  m2[1][1] + "))";
    res[1][0] = "(arctic_max (arctic_sum " + m1[1][0] + " " +  m2[0][0] + ") (arctic_sum " + m1[1][1] + " " +  m2[1][0] + "))";
    res[1][1] = "(arctic_max (arctic_sum " + m1[1][0] + " " +  m2[0][1] + ") (arctic_sum " + m1[1][1] + " " +  m2[1][1] + "))";
    return res;
}

vector<vector<string>> mult_matrix_by_vector(vector<vector<string>> m1, vector<vector<string>> m2){
    vector<vector<string>> res = {{""}, {""}};
    res[0][0] = "(arctic_max (arctic_sum " + m1[0][0] + " " +  m2[0][0] + ") (arctic_sum " + m1[0][1] + " " +  m2[1][0] + "))";
    res[1][0] = "(arctic_max (arctic_sum " + m1[1][0] + " " +  m2[0][0] + ") (arctic_sum " + m1[1][1] + " " +  m2[1][0] + "))";
    return res;
}

vector<vector<string>> sum_of_vectors (vector<vector<string>> m1, vector<vector<string>> m2){
    vector<vector<string>> res = {{""}, {""}};
    res[0][0] = "(arctic_sum " + m1[0][0] + " " +  m2[0][0] + " )";
    res[1][0] = "(arctic_sum " + m1[1][0] + " " +  m2[1][0] + " )";
    return res;
}

std::string process_variables_x(vector<string> multipliers_for_x_left, vector <string> multipliers_for_x_right){
    string res;
    vector<vector<string>> res_matrix_left = make_matrix_from_elem(multipliers_for_x_left[0]);
    for (int i = 1; i < multipliers_for_x_left.size(); i++){
        res_matrix_left = mult_matrix_by_matrix(res_matrix_left, make_matrix_from_elem(multipliers_for_x_left[i]));
    }
    vector<vector<string>> res_matrix_right = make_matrix_from_elem(multipliers_for_x_right[0]);
    for (int i = 1; i < multipliers_for_x_right.size(); i++){
        res_matrix_right = mult_matrix_by_matrix(res_matrix_right, make_matrix_from_elem(multipliers_for_x_right[i]));
    }
    res += "(assert (and ";
    for (int i = 0; i < res_matrix_left.size(); i++){
        for (int j = 0; j < res_matrix_left[i].size(); j++){
            res += " (arctic_comp " + res_matrix_left[i][j] + " " + res_matrix_right[i][j] + ")";
        }
    }
    res += "))\n";
    return res;
}

string process_free_odds(map<string, vector<string>> multipliers_for_elems_left, map<string, vector<string>> multipliers_for_elems_right){
    string res = ""; 
    vector<vector<string>> res_vector_left;
    for (auto it = multipliers_for_elems_left.begin(); it != multipliers_for_elems_left.end(); ++it){
        if (it -> first != "x"){
            cout << "left_part" << it -> first << endl;
            vector<vector<string>> cur_vector_left = make_vector_from_elem(it -> first);
            for (int i = 0 ; i < (it -> second).size(); i++){
                cur_vector_left = mult_matrix_by_vector(make_matrix_from_elem((it -> second)[i]), cur_vector_left);
            }
            if (res_vector_left.size() == 0){
                res_vector_left = cur_vector_left;
            }else{
                res_vector_left = sum_of_vectors(cur_vector_left, res_vector_left);
            }
        }
    }
    vector<vector<string>> res_vector_right;
    for (auto it = multipliers_for_elems_right.begin(); it != multipliers_for_elems_right.end(); ++it){
        if (it -> first != "x"){
            cout << "right_part" << it -> first << endl;
            vector<vector<string>> cur_vector_right = make_vector_from_elem(it -> first);
            for (int i = 0 ; i < (it -> second).size(); i++){
                cur_vector_right = mult_matrix_by_vector(make_matrix_from_elem((it -> second)[i]), cur_vector_right);
            }
            if (res_vector_right.size() == 0){
                res_vector_right = cur_vector_right;
            }else{
                res_vector_right = sum_of_vectors(cur_vector_right, res_vector_right);
            }
        }
    }
    res += "(assert (and ";
    for (int i = 0; i < res_vector_left.size(); i++){
        for (int j = 0; j < res_vector_left[i].size(); j++){
            res += " (arctic_comp " + res_vector_left[i][j] + " " + res_vector_right[i][j] + ")";
        }
    }
    res += "))\n";
    return res;
}

std::string declare_all_variables_by_smt2(map<string, vector<string>> multipliers_for_elems_left, map<string, vector<string>> multipliers_for_elems_right){
    string res;
    map<string , bool>declared_variables;
    for (int i = 0; i < multipliers_for_elems_left["x"].size(); i++){
        if (declared_variables[multipliers_for_elems_left["x"][i]] != true){
            declared_variables[multipliers_for_elems_left["x"][i]] = true;
        }
    }
    for (int i = 0; i < multipliers_for_elems_right["x"].size(); i++){
        if (declared_variables[multipliers_for_elems_right["x"][i]] != true){
            declared_variables[multipliers_for_elems_right["x"][i]] = true;
        }
    }
    for (auto it = multipliers_for_elems_left.begin(); it != multipliers_for_elems_left.end(); ++it){
        if (it -> first != "x" && declared_variables[it -> first] != true){
            declared_variables[it -> first] = true;
        }
    }
    for (auto it = multipliers_for_elems_right.begin(); it != multipliers_for_elems_right.end(); ++it){
        if (it -> first != "x" && declared_variables[it -> first] != true){
            declared_variables[it -> first] = true;
        }
    }
    for (auto it = declared_variables.begin(); it != declared_variables.end(); ++it){
        if ((it->first)[1] == '0'){
            for (int i = 0; i < 2; i++){
                for (int j = 0; j < 2; j++){
                    res += "(declare-fun " + it->first + to_string(i) + to_string(j) + " () Int)\n";
                }
            }
        }else{
            for (int i = 0; i < 2; i++){
                res += "(declare-fun " + it->first + to_string(i) + " () Int)\n";
            }
        }
    }
    return res;
}

std::string additionals_asserts(map<string, vector<string>> multipliers_for_elems_left,map<string, vector<string>> multipliers_for_elems_right){
    string res = "";
    map<string , bool>declared_variables;
    for (int i = 0; i < multipliers_for_elems_left["x"].size(); i++){
        if (declared_variables[multipliers_for_elems_left["x"][i]] != true){
            declared_variables[multipliers_for_elems_left["x"][i]] = true;
            res += "(assert (> " + multipliers_for_elems_left["x"][i] + "00" + " -1))\n";
            res += "(assert (>= " + multipliers_for_elems_left["x"][i] + "01" + " -1))\n";
            res += "(assert (>= " + multipliers_for_elems_left["x"][i] + "10" + " -1))\n";
            res += "(assert (>= " + multipliers_for_elems_left["x"][i] + "11" + " -1))\n";
        }
    }
    for (int i = 0; i < multipliers_for_elems_right["x"].size(); i++){
        if (declared_variables[multipliers_for_elems_right["x"][i]] != true){
            declared_variables[multipliers_for_elems_right["x"][i]] = true;
            res += "(assert (> " + multipliers_for_elems_right["x"][i] + "00" + " -1))\n";
            res += "(assert (>= " + multipliers_for_elems_right["x"][i] + "01" + " -1))\n";
            res += "(assert (>= " + multipliers_for_elems_right["x"][i] + "10" + " -1))\n";
            res += "(assert (>= " + multipliers_for_elems_right["x"][i] + "11" + " -1))\n";
        }
    }
    for (auto it = multipliers_for_elems_left.begin(); it != multipliers_for_elems_left.end(); ++it){
        if (it -> first != "x" && declared_variables[it -> first] != true){
            declared_variables[it -> first] = true;
            res += "(assert (or (> " + it->first + "0" + " -1) (and (= 0 " + it->first + "0" + ")  (= 0 " + it->first + "0" + " ))))\n";
            res += "(assert (>= " + it->first + "1" " -1 ))\n";
        }
    }
    for (auto it = multipliers_for_elems_right.begin(); it != multipliers_for_elems_right.end(); ++it){
        if (it -> first != "x" && declared_variables[it -> first] != true){
            declared_variables[it -> first] = true;
            res += "(assert (or (> " + it->first + "0" + " -1) (and (= 0 " + it->first + "0" + ")  (= 0 " + it->first + "0" + " ))))\n";
            res += "(assert (>= " + it->first + "1" " -1 ))\n";
        }
    }
    return res;
}

std::string form_smt2_instructions(map<string, vector<string>> multipliers_for_elems_left,map<string, vector<string>> multipliers_for_elems_right){
    std::string res = "(set-logic QF_NIA)\n";
    res += "(define-fun arctic_max ((a Int) (b Int)) Int (ite (>= a b)a b))\n";
    res += "(define-fun arctic_sum ((a Int) (b Int)) Int (ite (and (> a -1)  (> b -1)) (+ a b) (ite (<= a -1) b a )))\n";
    res += "(define-fun arctic_comp ((a Int) (b Int)) Bool (ite (or (> a b) (and (<= a -1) (<= b -1))) true false))\n";
    res += declare_all_variables_by_smt2(multipliers_for_elems_left, multipliers_for_elems_right);
    res += additionals_asserts(multipliers_for_elems_left, multipliers_for_elems_right);
    res += process_variables_x(multipliers_for_elems_left["x"], multipliers_for_elems_right["x"]);
    res += process_free_odds(multipliers_for_elems_left, multipliers_for_elems_right);
    res += "(check-sat)\n(get-model)\n";
    return res;
}

struct ParseTreeNode{
    string val;
    ParseTreeNode* left;
    ParseTreeNode* right;
    ParseTreeNode():val(""), left(nullptr), right(nullptr){}
    ParseTreeNode(string _val):val(_val), left(nullptr), right(nullptr){}
};

void fill_parse_tree(ParseTreeNode* root, string expr){
    if (expr[0] == 'x'){
        root->left = new ParseTreeNode("x");
    }else{
        string elem = "";
        elem += expr[0];
        root->left = new ParseTreeNode(elem + "0");
        root->right = new ParseTreeNode(elem + "1");
        return fill_parse_tree(root->left, expr.substr(expr.find('(') + 1, expr.find(')') - expr.find('(')));
    }
};

ParseTreeNode* make_parse_tree (string expr){
    ParseTreeNode* res = new ParseTreeNode();
    fill_parse_tree(res, expr);
    return res;
}

map<string, vector<string>> define_multipliers_for_elems(ParseTreeNode* parse_tree){
    vector<string>list_multipliers = {};
    map<string, vector<string>>multipliers_for_elems;
    while(parse_tree -> left -> val != "x"){
        multipliers_for_elems[parse_tree -> right -> val] = list_multipliers;
        list_multipliers.push_back(parse_tree -> left -> val);
        parse_tree = parse_tree -> left;
    }
    multipliers_for_elems["x"] = list_multipliers;
    return multipliers_for_elems;
}

string delete_brackets(string str){
    string res;
    for (int i = 0; i < str.length(); i++){
        if (str[i] != '(' && str[i] != ')'){
            res += str[i];
        }
    }
    return res;
}

string delete_spaces(string str){
    string res;
    for (int i = 0; i < str.length(); i++){
        if (str[i] != ' '){
            res += str[i];
        }
    }
    return res;
}

int main() {
    cout << "пример ввода f(g(x)) -> f(x)" << endl;
    string input_str, left_expr, right_expr;
    getline(cin, input_str);
    input_str = delete_spaces(input_str);
    left_expr = input_str.substr(0, input_str.find("->"));
    right_expr = input_str.substr(input_str.find("->") + 2);
    
    cout << left_expr << " " << right_expr << "\n";
    map<string, vector<string>> multipliers_for_elems_left = define_multipliers_for_elems(make_parse_tree(left_expr));
    map<string, vector<string>> multipliers_for_elems_right = define_multipliers_for_elems(make_parse_tree(right_expr));
    ofstream out("lab1.smt2", ios_base::trunc);
    out << form_smt2_instructions(multipliers_for_elems_left, multipliers_for_elems_right);
    out.close();
    return 0;
}



