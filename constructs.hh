#ifndef HWC_SYNTAX_TREE_H
#define HWC_SYNTAX_TREE_H

#include <list>
#include <string>
#include <memory>

struct construct{ virtual ~construct() = default; };

/**
 * 该部分数据结构是用于构造语法“树”的。
 * 
 * 该部分内容来自于 “SysY 语言的文法”，并按如下约定做修改
 * 0. 为identifier进行一层嵌套；其他都分装为同一个类型；
 * 1. 将类似于 A -> Ab | c 的结构表示为列表
 * 2. 非终结符号 NonTerminal 的结构体名称为 non_terminal_t
 * 3. 所有类的最终父类是 construct, 
 *    取自术语 language construct（语言构造/语言元素）
 * 4. 如果有一个产生式 A -> alpha | beta | ... 
 *    那么有结构体 a_x_t, a_y_t, 他们都继承自 a_t。
 *    x, y 是适当的名字
 *    在构造的时候，无需构造 a_t. 
 * 5. 如果有一个产生式是 A -> alpha(beta|gamma) 的格式，则拆开。
 *     给 beta|gamma 命名以适当的名字。之后参照规则 4 处理
 * 6. 例外情况会给出注释。
 * 
 * 关于 shared_ptr：
 * 经过某些优化的语法树可能呈现图结构，但该图应当是DAG。
 * 基于语法“树”中不会出现环的假设，这里使用 shared_ptr 智能指针
 * 
 **/

struct construct;

struct comp_unit_t;
struct comp_unit_item_t; // Decl | FuncDef
struct comp_unit_item_decl_t; 
struct comp_unit_item_func_def_t; 

struct decl_t;
struct decl_const_decl_t;
struct decl_var_decl_t;

struct const_decl_t;

struct const_def_t;
struct const_init_val_t;
struct const_init_val_scalar_t;
struct const_init_val_array_t;

struct var_decl_t;
struct var_def_t;
struct var_def_only_t;
struct var_def_init_t;

struct init_val_t;
struct init_val_scalar_t;
struct init_val_array_t;

struct func_def_t;

struct func_f_params_t;
struct func_f_param_t;

struct block_t;
struct block_item_t;
struct block_item_decl_t;
struct block_item_stmt_t;

struct stmt_t;
struct stmt_assign_t;
struct stmt_exp_t;
struct stmt_block_t;
struct stmt_if_t;
struct stmt_while_t;
struct stmt_break_t;
struct stmt_continue_t;
struct stmt_return_t;

struct expr;
struct l_val_t;
struct number_t;
struct func_call_t;
struct func_r_params_t;

struct expr;

struct terminal_wrapper: public construct{};

// 下面是终结符号的包装
// 受限于LALR(1)的能力（是吗？），func_type_t 和 b_type_t 不做区分，放在语义分析（TODO）部分判断
struct b_type_t: public terminal_wrapper {
    enum Type {VOID, INT};
    b_type_t(const Type& type):type(type){}
    Type type;
};

struct ident_t: public terminal_wrapper {
    ident_t(const std::string& name):name(name){}
    std::string name;
};

// ; , { } ( ) 等和语义无关的符号未列出
struct operator_t: public terminal_wrapper {
    enum Type { 
        POSITIVE,
        NEGATIVE,
        
        PLUS,
        MINUS,
        MULTIPLY,
        DIVIDE,
        MODULE,

        GREATER,
        LESS,
        GREATER_EQUAL,
        LESS_EQUAL,
        EQUAL,
        NOT_EQUAL,

        LOGICAL_NOT,
        LOGICAL_AND,
        LOGICAL_OR,
    };
    operator_t(const Type& type): type(type) {}
    Type type;
};

// 非终结符号类型的定义

template <typename T>
using ptr_list_of = std::list<std::shared_ptr<T>>;

struct comp_unit_t: public construct {
    ptr_list_of<comp_unit_item_t> children;
};

struct comp_unit_item_t: public construct {}; // Decl | FuncDef
struct comp_unit_item_decl_t: public comp_unit_item_t {
    comp_unit_item_decl_t(const std::shared_ptr<decl_t>& decl): decl(decl){}
    std::shared_ptr<decl_t> decl;
}; 
struct comp_unit_item_func_def_t: public comp_unit_item_t {
    comp_unit_item_func_def_t(const std::shared_ptr<func_def_t>& func_def): func_def(func_def){}
    std::shared_ptr<func_def_t> func_def;
}; 

struct decl_t: public construct {};
struct decl_const_decl_t: public decl_t {
    decl_const_decl_t(const std::shared_ptr<const_decl_t>& const_decl): const_decl(const_decl){}
    std::shared_ptr<const_decl_t> const_decl;
};
struct decl_var_decl_t: public decl_t {
    decl_var_decl_t(const std::shared_ptr<var_decl_t>& var_decl): var_decl(var_decl){}
    std::shared_ptr<var_decl_t> var_decl;
};

struct const_decl_t: public construct {
    const_decl_t() {}
    const_decl_t(const std::shared_ptr<b_type_t>& b_type, ptr_list_of<const_def_t> const_defs):
        b_type(b_type), const_defs(const_defs) {}
    std::shared_ptr<b_type_t> b_type;
    ptr_list_of<const_def_t> const_defs;
};

struct const_def_t: public construct {
    const_def_t(    
        const std::string ident,
        const ptr_list_of<expr> array_dims,
        const std::shared_ptr<const_init_val_t> const_init_val
    ):ident(ident), array_dims(array_dims), const_init_val(const_init_val){}
    std::string ident;
    ptr_list_of<expr> array_dims;
    std::shared_ptr<const_init_val_t> const_init_val;    
};

struct const_init_val_t: public construct {};
struct const_init_val_scalar_t: public const_init_val_t {
    const_init_val_scalar_t(const std::shared_ptr<expr>& const_exp): const_exp(const_exp){}
    std::shared_ptr<expr> const_exp;
};
struct const_init_val_array_t: public const_init_val_t {
    const_init_val_array_t(const ptr_list_of<const_init_val_t>& array_elements):array_elements(array_elements){}
    ptr_list_of<const_init_val_t> array_elements;
};

struct var_decl_t: public construct {
    var_decl_t(
        const std::shared_ptr<b_type_t>& b_type,
        const ptr_list_of<var_def_t>& var_defs
    ):b_type(b_type), var_defs(var_defs){}
    std::shared_ptr<b_type_t> b_type;
    ptr_list_of<var_def_t> var_defs;
};

struct var_def_t: public construct {};
struct var_def_only_t: public var_def_t {
    var_def_only_t( 
        const std::string& ident,
        const ptr_list_of<expr>& array_dims
    ):ident(ident), array_dims(array_dims){}
    std::string ident;
    ptr_list_of<expr> array_dims;
};
struct var_def_init_t: public var_def_t {
    var_def_init_t( 
        const std::string& ident,
        const ptr_list_of<expr>& array_dims,
        const std::shared_ptr<init_val_t>& init_val
    ):ident(ident), array_dims(array_dims), init_val(init_val){}
    std::string ident;
    ptr_list_of<expr> array_dims;
    std::shared_ptr<init_val_t> init_val;
};

struct init_val_t: public construct {};
struct init_val_scalar_t: public init_val_t {
    init_val_scalar_t(const std::shared_ptr<expr>& exp):exp(exp){}
    std::shared_ptr<expr> exp;
};
struct init_val_array_t: public init_val_t {
    init_val_array_t(const ptr_list_of<init_val_t>& array_elements):array_elements(array_elements){}
    ptr_list_of<init_val_t> array_elements; 
}; 



struct func_def_t: public construct {
    std::shared_ptr<b_type_t> func_type;
    std::string ident;
    std::shared_ptr<func_f_params_t> params;
    std::shared_ptr<block_t> block; 
};

struct func_f_params_t: public construct {
    func_f_params_t(){}
    func_f_params_t(const ptr_list_of<func_f_param_t>& params):params(params){}
    ptr_list_of<func_f_param_t> params;
};
struct func_f_param_t: public construct {
    std::shared_ptr<b_type_t> b_type;
    std::string ident;
    // 做高维的留空，填 nullptr
    ptr_list_of<expr> array_dims;
};

struct block_t: public construct {
    block_t(const ptr_list_of<block_item_t>& children): children(children){}
    ptr_list_of<block_item_t> children;
};
struct block_item_t: public construct {};
struct block_item_decl_t: public block_item_t {
    block_item_decl_t(const std::shared_ptr<decl_t>& decl): decl(decl){}
    std::shared_ptr<decl_t> decl;
};
struct block_item_stmt_t: public block_item_t {
    block_item_stmt_t(const std::shared_ptr<stmt_t>& stmt): stmt(stmt){}
    std::shared_ptr<stmt_t> stmt;
};

struct stmt_t: public construct {};

struct stmt_assign_t: public stmt_t {
    stmt_assign_t(
        const std::shared_ptr<l_val_t>& l_val,
        const std::shared_ptr<expr>& exp
    ): l_val(l_val), exp(exp) {}
    std::shared_ptr<l_val_t> l_val;
    std::shared_ptr<expr> exp;
};

struct stmt_exp_t: public stmt_t {
    stmt_exp_t(const std::shared_ptr<expr>& exp): exp(exp) {}
    std::shared_ptr<expr> exp;
};

struct stmt_block_t: public stmt_t {
    stmt_block_t(const std::shared_ptr<block_t>& block): block(block) {}
    std::shared_ptr<block_t> block;
};

struct stmt_if_t: public stmt_t {
    stmt_if_t(
        const std::shared_ptr<expr>& cond,
        const std::shared_ptr<stmt_t>& stmt_if_true,
        const std::shared_ptr<stmt_t>& stmt_if_false
    ): cond(cond), stmt_if_true(stmt_if_true), stmt_if_false(stmt_if_false) {}
    std::shared_ptr<expr> cond;
    std::shared_ptr<stmt_t> stmt_if_true;
    std::shared_ptr<stmt_t> stmt_if_false;
};

struct stmt_while_t: public stmt_t {
    stmt_while_t(
        const std::shared_ptr<expr>& cond,
        const std::shared_ptr<stmt_t>& stmt
    ): cond(cond), stmt(stmt) {}
    std::shared_ptr<expr> cond;
    std::shared_ptr<stmt_t> stmt;
};

struct stmt_break_t: public stmt_t {};

struct stmt_continue_t: public stmt_t {};

struct stmt_return_t: public stmt_t {
    stmt_return_t(const std::shared_ptr<expr>& exp): exp(exp) {}
    std::shared_ptr<expr> exp;
};


/**
 * Expression
 * 
 * 关系、算术与逻辑表达式数据结构
 * 因为在语法定义里面，没有明确区分整数类型和浮点类型，所以这么做
 * 非 cond context 看到关系与逻辑表达式直接报错就好 ?
 * 分析关系逻辑表达式的时候，当作正常短路逻辑来做，需要的时候求值
 **/

struct expr: public construct {
};

struct logical_not_expr: public expr {
    std::shared_ptr<expr> src;
    logical_not_expr(std::shared_ptr<expr> src): src(src) {}
};

struct negative_expr: public expr {
    std::shared_ptr<expr> src;
    negative_expr(std::shared_ptr<expr> src): src(src) {}
};

struct binary_expr: public expr {
    operator_t op;
    std::shared_ptr<expr> src1;
    std::shared_ptr<expr> src2;
    binary_expr(
        operator_t op,
        std::shared_ptr<expr> src1,
        std::shared_ptr<expr> src2
    ) : op(op), src1(src1), src2(src2) {
    }
};

struct l_val_t: public expr {
    l_val_t(
        std::string ident,
        const ptr_list_of<expr>& exps
    ): ident(ident), exps(exps) {}
    std::string ident;
    ptr_list_of<expr> exps;
};

struct number_literal_t: public expr {
    number_literal_t(int32_t value): value(value) {}
    number_literal_t(){}
    int32_t value;
};

struct func_call_t: public expr {
    func_call_t(
        const std::string& ident,
        const std::shared_ptr<func_r_params_t>& params
    ): ident(ident), params(params){}
    std::string ident;
    std::shared_ptr<func_r_params_t> params;
};


struct func_r_params_t: public construct {
    func_r_params_t(){}
    func_r_params_t(const ptr_list_of<expr>& exps): exps(exps) {}
    ptr_list_of<expr> exps;
};




#endif