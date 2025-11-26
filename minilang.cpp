// minilang.cpp
// Single-file MiniLang: lexer, parser, AST, semantic checks, constant-folding, TAC, interpreter, CLI
// Compile: g++ -std=c++17 minilang.cpp -O2 -o minilang

#include <bits/stdc++.h>
using namespace std;

// ============================================================================
// PHASE 0: LANGUAGE SPECIFICATION
// ============================================================================
static const char *MINILANG_SPEC = R"SPEC(
[MiniLang specification remains the same...]
)SPEC";

// ============================================================================
// PHASE 1: LEXICAL ANALYSIS - Token Definitions
// ============================================================================
enum class TokenType {
        END,   // EOF
        INT_LIT, IDENT,
        PLUS, MINUS, MUL, DIV, MOD,
        ASSIGN, EQ, NEQ, LT, GT, LTE, GTE,
        LPAREN, RPAREN, LBRACE, RBRACE, SEMI,
        KW_PRINT, KW_IF, KW_ELSE, KW_WHILE,
};

string tokenTypeName(TokenType t) {
    // [Implementation remains the same...]
}

struct Token { 
    TokenType type; 
    string text; 
    long long intVal; 
    int line;
    Token(TokenType t=TokenType::END, string s="", int l=1): type(t), text(s), intVal(0), line(l) {} 
};

// ============================================================================
// LEXER IMPLEMENTATION
// ============================================================================
struct Lexer {
    string src; 
    size_t i=0; 
    int line=1;
    bool debug;
    
    Lexer(const string &s, bool dbg=false): src(s), debug(dbg) { 
        if(debug) cout << "[LEXER] Initialized with source length: " << src.length() << endl;
    }
    
    char peek(){ return i < src.size() ? src[i] : '\0'; }
    char get(){ return i < src.size() ? src[i++] : '\0'; }
    bool startswith(const string &pat){ return src.substr(i, pat.size()) == pat; }
    
    Token nextToken(){
        while(true){
            char c = peek();
            if(c=='\0') {
                if(debug) cout << "[LEXER] End of file reached" << endl;
                return Token(TokenType::END, "", line);
            }
            if(isspace(static_cast<unsigned char>(c))){ 
                if(c=='\n') line++; 
                get(); 
                continue; 
            }
            if(startswith("//")){ 
                if(debug) cout << "[LEXER] Skipping comment" << endl;
                while(peek() && peek()!='\n') get(); 
                continue; 
            }
            if(isdigit(static_cast<unsigned char>(c))){ 
                string s; 
                while(isdigit(static_cast<unsigned char>(peek()))) s.push_back(get()); 
                Token t(TokenType::INT_LIT, s, line); 
                t.intVal = stoll(s); 
                if(debug) cout << "[LEXER] Integer literal: " << s << " (value: " << t.intVal << ")" << endl;
                return t; 
            }
            if(isalpha(static_cast<unsigned char>(c)) || c=='_'){ 
                string s; 
                while(isalnum(static_cast<unsigned char>(peek())) || peek()=='_') s.push_back(get()); 
                Token t(TokenType::IDENT, s, line);
                if(s=="print") { t.type = TokenType::KW_PRINT; if(debug) cout << "[LEXER] Keyword: print" << endl; }
                else if(s=="if") { t.type = TokenType::KW_IF; if(debug) cout << "[LEXER] Keyword: if" << endl; }
                else if(s=="else") { t.type = TokenType::KW_ELSE; if(debug) cout << "[LEXER] Keyword: else" << endl; }
                else if(s=="while") { t.type = TokenType::KW_WHILE; if(debug) cout << "[LEXER] Keyword: while" << endl; }
                else { if(debug) cout << "[LEXER] Identifier: " << s << endl; }
                return t; 
            }
            if(startswith("==")){ i+=2; if(debug) cout << "[LEXER] Operator: ==" << endl; return Token(TokenType::EQ, "==", line); }
            if(startswith("!=")){ i+=2; if(debug) cout << "[LEXER] Operator: !=" << endl; return Token(TokenType::NEQ, "!=", line); }
            if(startswith("<=")){ i+=2; if(debug) cout << "[LEXER] Operator: <=" << endl; return Token(TokenType::LTE, "<=", line); }
            if(startswith(">=")){ i+=2; if(debug) cout << "[LEXER] Operator: >=" << endl; return Token(TokenType::GTE, ">=", line); }
            
            char ch = get();
            Token result(TokenType::END, "", line);
            switch(ch){
                case '+': result = Token(TokenType::PLUS, "+", line); break;
                case '-': result = Token(TokenType::MINUS, "-", line); break;
                case '*': result = Token(TokenType::MUL, "*", line); break;
                case '/': result = Token(TokenType::DIV, "/", line); break;
                case '%': result = Token(TokenType::MOD, "%", line); break;
                case '=': result = Token(TokenType::ASSIGN, "=", line); break;
                case '<': result = Token(TokenType::LT, "<", line); break;
                case '>': result = Token(TokenType::GT, ">", line); break;
                case '(' : result = Token(TokenType::LPAREN, "(", line); break;
                case ')' : result = Token(TokenType::RPAREN, ")", line); break;
                case '{' : result = Token(TokenType::LBRACE, "{", line); break;
                case '}' : result = Token(TokenType::RBRACE, "}", line); break;
                case ';' : result = Token(TokenType::SEMI, ";", line); break;
                default: cerr<<"[LEXER ERROR] Line " << line << ": unexpected char '"<<ch<<"'\n"; exit(1);
            }
            if(debug && result.type != TokenType::END) {
                cout << "[LEXER] Token: " << tokenTypeName(result.type) << " '" << result.text << "'" << endl;
            }
            return result;
        }
    }
};

// ============================================================================
// PHASE 2: SYNTAX ANALYSIS - Abstract Syntax Tree Definitions
// ============================================================================
struct NodeBase { virtual ~NodeBase(){} };

struct Expr : NodeBase { 
    virtual long long eval(map<string,long long>&env) = 0; 
    virtual string toString() const = 0;
};

struct Stmt : NodeBase { 
    virtual void exec(map<string,long long>&env) = 0; 
    virtual string toString() const = 0;
};

// Expression Nodes
struct IntLit : Expr { 
    long long v; 
    IntLit(long long vv): v(vv){} 
    long long eval(map<string,long long>&) override { return v; } 
    string toString() const override { return "IntLit(" + to_string(v) + ")"; }
};

struct VarExpr : Expr { 
    string name; 
    VarExpr(const string &n): name(n){} 
    long long eval(map<string,long long>&env) override { 
        if(env.find(name)==env.end()){ 
            cerr<<"[RUNTIME ERROR] Use of undefined variable '"<<name<<"'\n"; 
            exit(1);
        } 
        return env[name]; 
    } 
    string toString() const override { return "VarExpr(" + name + ")"; }
};

struct Binary : Expr { 
    string op; 
    unique_ptr<Expr> a,b; 
    Binary(const string &op_, unique_ptr<Expr> a_, unique_ptr<Expr> b_): op(op_), a(move(a_)), b(move(b_)){} 
    
    long long eval(map<string,long long>&env) override { 
        long long A=a->eval(env), B=b->eval(env); 
        if(op=="+") return A+B; 
        if(op=="-") return A-B; 
        if(op=="*") return A*B; 
        if(op=="/"){ 
            if(B==0){ 
                cerr<<"[RUNTIME ERROR] Division by zero\n"; 
                exit(1);
            } 
            return A/B; 
        } 
        if(op=="%") return A%B; 
        if(op=="==") return A==B; 
        if(op=="!=") return A!=B; 
        if(op=="<") return A<B; 
        if(op==">") return A>B; 
        if(op=="<=") return A<=B; 
        if(op==">=") return A>=B; 
        cerr<<"[RUNTIME ERROR] Unknown operator "<<op<<"\n"; 
        exit(1); 
    } 
    
    string toString() const override { 
        return "Binary(" + op + ", " + a->toString() + ", " + b->toString() + ")"; 
    }
};

// Statement Nodes
struct PrintStmt : Stmt { 
    unique_ptr<Expr> e; 
    PrintStmt(unique_ptr<Expr> e_): e(move(e_)){} 
    void exec(map<string,long long>&env) override { 
        cout << e->eval(env) << "\n"; 
    } 
    string toString() const override { 
        return "PrintStmt(" + e->toString() + ")"; 
    }
};

struct AssignStmt : Stmt { 
    string name; 
    unique_ptr<Expr> e; 
    AssignStmt(const string &n, unique_ptr<Expr> e_): name(n), e(move(e_)){} 
    void exec(map<string,long long>&env) override { 
        long long val = e->eval(env); 
        env[name]=val; 
    } 
    string toString() const override { 
        return "AssignStmt(" + name + ", " + e->toString() + ")"; 
    }
};

struct BlockStmt : Stmt { 
    vector<unique_ptr<Stmt>> stmts; 
    void exec(map<string,long long>&env) override { 
        for(auto &s: stmts) s->exec(env); 
    } 
    string toString() const override { 
        string result = "BlockStmt[\n";
        for(auto &s: stmts) result += "  " + s->toString() + "\n";
        return result + "]";
    }
};

struct IfStmt : Stmt { 
    unique_ptr<Expr> cond; 
    unique_ptr<BlockStmt> thenBlock; 
    unique_ptr<BlockStmt> elseBlock; 
    IfStmt(unique_ptr<Expr> c, unique_ptr<BlockStmt> t, unique_ptr<BlockStmt> e): 
        cond(move(c)), thenBlock(move(t)), elseBlock(move(e)){} 
    void exec(map<string,long long>&env) override { 
        if(cond->eval(env)) thenBlock->exec(env); 
        else if(elseBlock) elseBlock->exec(env); 
    } 
    string toString() const override { 
        string result = "IfStmt(" + cond->toString() + ",\n  THEN: " + thenBlock->toString();
        if(elseBlock) result += ",\n  ELSE: " + elseBlock->toString();
        return result + ")";
    }
};

struct WhileStmt : Stmt { 
    unique_ptr<Expr> cond; 
    unique_ptr<BlockStmt> body; 
    WhileStmt(unique_ptr<Expr> c, unique_ptr<BlockStmt> b): cond(move(c)), body(move(b)){} 
    void exec(map<string,long long>&env) override { 
        while(cond->eval(env)) body->exec(env); 
    } 
    string toString() const override { 
        return "WhileStmt(" + cond->toString() + ", " + body->toString() + ")"; 
    }
};

// ============================================================================
// PHASE 2: SYNTAX ANALYSIS - Parser Implementation
// ============================================================================
struct Parser {
    Lexer lex; 
    Token cur; 
    bool debug;
    
    Parser(const string &s, bool dbg=false): lex(s, dbg), debug(dbg) { 
        cur = lex.nextToken(); 
        if(debug) cout << "[PARSER] Initialized, first token: " << tokenTypeName(cur.type) << endl;
    }
    
    void eat(TokenType t){ 
        if(cur.type==t) {
            if(debug) cout << "[PARSER] Consumed token: " << tokenTypeName(t) << endl;
            cur = lex.nextToken(); 
        } else { 
            cerr<<"[PARSER ERROR] Line " << cur.line << ": expected "<<tokenTypeName(t)<<" but got "<<tokenTypeName(cur.type)<<" ('"<<cur.text<<"')\n"; 
            exit(1); 
        } 
    }
    
    unique_ptr<BlockStmt> parseProgram(){ 
        if(debug) cout << "[PARSER] Starting program parsing" << endl;
        auto root = make_unique<BlockStmt>(); 
        while(cur.type!=TokenType::END) {
            root->stmts.push_back(parseStatement());
        }
        if(debug) {
            cout << "[PARSER] Program parsing complete. AST:" << endl;
            cout << root->toString() << endl;
        }
        return root; 
    }
    
    unique_ptr<Stmt> parseStatement(){
        if(debug) cout << "[PARSER] Parsing statement, current token: " << tokenTypeName(cur.type) << endl;
        
        if(cur.type==TokenType::KW_PRINT){ 
            if(debug) cout << "[PARSER] Found print statement" << endl;
            eat(TokenType::KW_PRINT); 
            eat(TokenType::LPAREN); 
            auto e=parseExpr(); 
            eat(TokenType::RPAREN); 
            eat(TokenType::SEMI); 
            return make_unique<PrintStmt>(move(e)); 
        }
        if(cur.type==TokenType::IDENT){ 
            string name=cur.text; 
            if(debug) cout << "[PARSER] Found assignment to variable: " << name << endl;
            eat(TokenType::IDENT); 
            eat(TokenType::ASSIGN); 
            auto e=parseExpr(); 
            eat(TokenType::SEMI); 
            return make_unique<AssignStmt>(name, move(e)); 
        }
        if(cur.type==TokenType::KW_IF){ 
            if(debug) cout << "[PARSER] Found if statement" << endl;
            eat(TokenType::KW_IF); 
            eat(TokenType::LPAREN); 
            auto cond=parseExpr(); 
            eat(TokenType::RPAREN); 
            auto thenB=parseBlock(); 
            unique_ptr<BlockStmt> elseB=nullptr; 
            if(cur.type==TokenType::KW_ELSE){ 
                if(debug) cout << "[PARSER] Found else clause" << endl;
                eat(TokenType::KW_ELSE); 
                elseB=parseBlock(); 
            } 
            return make_unique<IfStmt>(move(cond), move(thenB), move(elseB)); 
        }
        if(cur.type==TokenType::KW_WHILE){ 
            if(debug) cout << "[PARSER] Found while statement" << endl;
            eat(TokenType::KW_WHILE); 
            eat(TokenType::LPAREN); 
            auto cond=parseExpr(); 
            eat(TokenType::RPAREN); 
            auto body=parseBlock(); 
            return make_unique<WhileStmt>(move(cond), move(body)); 
        }
        if(cur.type==TokenType::LBRACE) {
            if(debug) cout << "[PARSER] Found block statement" << endl;
            return parseBlock();
        }
        cerr<<"[PARSER ERROR] Unexpected token "<<tokenTypeName(cur.type)<<" ('"<<cur.text<<"')\n"; 
        exit(1);
    }
    
    unique_ptr<BlockStmt> parseBlock(){ 
        eat(TokenType::LBRACE); 
        auto blk=make_unique<BlockStmt>(); 
        while(cur.type!=TokenType::RBRACE) {
            blk->stmts.push_back(parseStatement());
        }
        eat(TokenType::RBRACE); 
        return blk; 
    }
    
    unique_ptr<Expr> parseExpr(){ return parseEquality(); }
    
    unique_ptr<Expr> parseEquality(){ 
        auto left=parseComparison(); 
        while(cur.type==TokenType::EQ||cur.type==TokenType::NEQ){ 
            string op=cur.text; 
            if(debug) cout << "[PARSER] Equality operator: " << op << endl;
            eat(cur.type); 
            auto right=parseComparison(); 
            left=make_unique<Binary>(op, move(left), move(right)); 
        } 
        return left; 
    }
    
    unique_ptr<Expr> parseComparison(){ 
        auto left=parseTerm(); 
        while(cur.type==TokenType::LT||cur.type==TokenType::GT||cur.type==TokenType::LTE||cur.type==TokenType::GTE){ 
            string op=cur.text; 
            if(debug) cout << "[PARSER] Comparison operator: " << op << endl;
            eat(cur.type); 
            auto right=parseTerm(); 
            left=make_unique<Binary>(op, move(left), move(right)); 
        } 
        return left; 
    }
    
    unique_ptr<Expr> parseTerm(){ 
        auto left=parseFactor(); 
        while(cur.type==TokenType::PLUS||cur.type==TokenType::MINUS){ 
            string op=cur.text; 
            if(debug) cout << "[PARSER] Term operator: " << op << endl;
            eat(cur.type); 
            auto right=parseFactor(); 
            left=make_unique<Binary>(op, move(left), move(right)); 
        } 
        return left; 
    }
    
    unique_ptr<Expr> parseFactor(){ 
        auto left=parseUnary(); 
        while(cur.type==TokenType::MUL||cur.type==TokenType::DIV||cur.type==TokenType::MOD){ 
            string op=cur.text; 
            if(debug) cout << "[PARSER] Factor operator: " << op << endl;
            eat(cur.type); 
            auto right=parseUnary(); 
            left=make_unique<Binary>(op, move(left), move(right)); 
        } 
        return left; 
    }
    
    unique_ptr<Expr> parseUnary(){ 
        if(cur.type==TokenType::PLUS){ 
            if(debug) cout << "[PARSER] Unary plus" << endl;
            eat(TokenType::PLUS); 
            return parseUnary(); 
        } else if(cur.type==TokenType::MINUS){ 
            if(debug) cout << "[PARSER] Unary minus" << endl;
            eat(TokenType::MINUS); 
            auto r=parseUnary(); 
            return make_unique<Binary>(string("-"), make_unique<IntLit>(0), move(r)); 
        } else return parsePrimary(); 
    }
    
    unique_ptr<Expr> parsePrimary(){ 
        if(cur.type==TokenType::INT_LIT){ 
            long long v=cur.intVal; 
            if(debug) cout << "[PARSER] Integer literal: " << v << endl;
            eat(TokenType::INT_LIT); 
            return make_unique<IntLit>(v); 
        } else if(cur.type==TokenType::IDENT){ 
            string name=cur.text; 
            if(debug) cout << "[PARSER] Variable: " << name << endl;
            eat(TokenType::IDENT); 
            return make_unique<VarExpr>(name); 
        } else if(cur.type==TokenType::LPAREN){ 
            if(debug) cout << "[PARSER] Parenthesized expression" << endl;
            eat(TokenType::LPAREN); 
            auto e=parseExpr(); 
            eat(TokenType::RPAREN); 
            return e; 
        } 
        cerr<<"[PARSER ERROR] Unexpected primary token\n"; 
        exit(1); 
    }
};

// ============================================================================
// PHASE 3: SEMANTIC ANALYSIS
// ============================================================================
void semanticCheckBlock(BlockStmt* blk, set<string>& defined, int depth=0){
    string indent(depth*2, ' ');
    if(depth == 0) cout << "[SEMANTIC] Starting semantic analysis..." << endl;
    
    for(auto &s: blk->stmts){
        if(auto as = dynamic_cast<AssignStmt*>(s.get())){
            cout << indent << "[SEMANTIC] Checking assignment to: " << as->name << endl;
            
            function<void(Expr*)> findVars = [&](Expr* e){ 
                if(!e) return; 
                if(auto ve = dynamic_cast<VarExpr*>(e)){ 
                    if(defined.find(ve->name)==defined.end()){ 
                        cerr<<"[SEMANTIC ERROR] Variable '"<<ve->name<<"' used before assignment\n"; 
                        exit(1);
                    } else {
                        cout << indent << "[SEMANTIC] Valid use of variable: " << ve->name << endl;
                    }
                } else if(auto b = dynamic_cast<Binary*>(e)){ 
                    findVars(b->a.get()); 
                    findVars(b->b.get()); 
                } 
            };
            
            findVars(as->e.get()); 
            defined.insert(as->name);
            cout << indent << "[SEMANTIC] Variable defined: " << as->name << endl;
            
        } else if(auto ifs = dynamic_cast<IfStmt*>(s.get())){
            cout << indent << "[SEMANTIC] Checking if statement condition" << endl;
            
            function<void(Expr*)> findVars = [&](Expr* e){ 
                if(!e) return; 
                if(auto ve = dynamic_cast<VarExpr*>(e)){ 
                    if(defined.find(ve->name)==defined.end()){ 
                        cerr<<"[SEMANTIC ERROR] Variable '"<<ve->name<<"' used before assignment\n"; 
                        exit(1);
                    }
                } else if(auto b = dynamic_cast<Binary*>(e)){ 
                    findVars(b->a.get()); 
                    findVars(b->b.get()); 
                } 
            };
            
            findVars(ifs->cond.get());
            
            // check then and else with copies of defined
            set<string> thenDef = defined; 
            cout << indent << "[SEMANTIC] Checking then block..." << endl;
            semanticCheckBlock(ifs->thenBlock.get(), thenDef, depth+1);
            
            if(ifs->elseBlock){ 
                set<string> elseDef = defined; 
                cout << indent << "[SEMANTIC] Checking else block..." << endl;
                semanticCheckBlock(ifs->elseBlock.get(), elseDef, depth+1); 
            }
            
        } else if(auto wh = dynamic_cast<WhileStmt*>(s.get())){
            cout << indent << "[SEMANTIC] Checking while statement condition" << endl;
            
            function<void(Expr*)> findVars = [&](Expr* e){ 
                if(!e) return; 
                if(auto ve = dynamic_cast<VarExpr*>(e)){ 
                    if(defined.find(ve->name)==defined.end()){ 
                        cerr<<"[SEMANTIC ERROR] Variable '"<<ve->name<<"' used before assignment\n"; 
                        exit(1);
                    }
                } else if(auto b = dynamic_cast<Binary*>(e)){ 
                    findVars(b->a.get()); 
                    findVars(b->b.get()); 
                } 
            };
            
            findVars(wh->cond.get());
            set<string> bodyDef = defined; 
            cout << indent << "[SEMANTIC] Checking while loop body..." << endl;
            semanticCheckBlock(wh->body.get(), bodyDef, depth+1);
            
        } else if(auto blk2 = dynamic_cast<BlockStmt*>(s.get())){
            cout << indent << "[SEMANTIC] Checking nested block..." << endl;
            semanticCheckBlock(blk2, defined, depth+1);
            
        } else if(dynamic_cast<PrintStmt*>(s.get())){
            auto ps = static_cast<PrintStmt*>(s.get()); 
            cout << indent << "[SEMANTIC] Checking print statement" << endl;
            function<void(Expr*)> findVars = [&](Expr* e){ 
                if(!e) return; 
                if(auto ve = dynamic_cast<VarExpr*>(e)){ 
                    if(defined.find(ve->name)==defined.end()){ 
                        cerr<<"[SEMANTIC ERROR] Variable '"<<ve->name<<"' used before assignment\n"; 
                        exit(1);
                    } else {
                        cout << indent << "[SEMANTIC] Valid use in print: " << ve->name << endl;
                    }
                } else if(auto b = dynamic_cast<Binary*>(e)){ 
                    findVars(b->a.get()); 
                    findVars(b->b.get()); 
                } 
            }; 
            findVars(ps->e.get());
        }
    }
    
    if(depth == 0) cout << "[SEMANTIC] Semantic analysis completed successfully!" << endl;
}

// ============================================================================
// PHASE 5: OPTIMIZATION - Constant Folding
// ============================================================================
unique_ptr<Expr> foldExpr(unique_ptr<Expr> e){
    if(auto b = dynamic_cast<Binary*>(e.get())){
        b->a = foldExpr(move(b->a));
        b->b = foldExpr(move(b->b));
        if(auto A = dynamic_cast<IntLit*>(b->a.get())){
            if(auto B = dynamic_cast<IntLit*>(b->b.get())){
                long long av = A->v, bv = B->v; 
                long long r=0; 
                bool ok=true;
                if(b->op=="+") r = av + bv; 
                else if(b->op=="-") r = av - bv; 
                else if(b->op=="*") r = av * bv; 
                else if(b->op=="/"){ if(bv==0) ok=false; else r = av / bv; } 
                else if(b->op=="%") { if(bv==0) ok=false; else r = av % bv; } 
                else if(b->op=="==") r = av==bv; 
                else if(b->op=="!=") r = av!=bv; 
                else if(b->op=="<") r = av < bv; 
                else if(b->op==">") r = av > bv; 
                else if(b->op=="<=") r = av <= bv; 
                else if(b->op==">=") r = av >= bv; 
                else ok=false;
                
                if(ok) {
                    cout << "[OPTIMIZATION] Constant folded: " << av << " " << b->op << " " << bv << " = " << r << endl;
                    return make_unique<IntLit>(r);
                }
            }
        }
        return e;
    }
    return e;
}

void foldConstantsInBlock(BlockStmt* blk){
    cout << "[OPTIMIZATION] Starting constant folding..." << endl;
    for(auto &s : blk->stmts){
        if(auto as = dynamic_cast<AssignStmt*>(s.get())){ 
            as->e = foldExpr(move(as->e)); 
        }
        else if(auto ifs = dynamic_cast<IfStmt*>(s.get())){ 
            ifs->cond = foldExpr(move(ifs->cond)); 
            foldConstantsInBlock(ifs->thenBlock.get()); 
            if(ifs->elseBlock) foldConstantsInBlock(ifs->elseBlock.get()); 
        }
        else if(auto wh = dynamic_cast<WhileStmt*>(s.get())){ 
            wh->cond = foldExpr(move(wh->cond)); 
            foldConstantsInBlock(wh->body.get()); 
        }
        else if(auto blk2 = dynamic_cast<BlockStmt*>(s.get())) 
            foldConstantsInBlock(blk2);
        else if(auto ps = dynamic_cast<PrintStmt*>(s.get())) 
            ps->e = foldExpr(move(ps->e));
    }
    cout << "[OPTIMIZATION] Constant folding completed!" << endl;
}

// ============================================================================
// PHASE 4 & 6: INTERMEDIATE CODE GENERATION - Three Address Code
// ============================================================================
struct TACGen {
    vector<string> code; 
    int tmpCounter = 0;
    bool debug;
    
    TACGen(bool dbg=false): debug(dbg) {}
    
    string newTmp(){ 
        string tmp = string("t") + to_string(++tmpCounter);
        if(debug) cout << "[TAC] New temporary: " << tmp << endl;
        return tmp;
    }
    
    string genExpr(Expr* e){
        if(auto il = dynamic_cast<IntLit*>(e)){
            return to_string(il->v);
        } else if(auto ve = dynamic_cast<VarExpr*>(e)){
            return ve->name;
        } else if(auto b = dynamic_cast<Binary*>(e)){
            string A = genExpr(b->a.get()); 
            string B = genExpr(b->b.get()); 
            string t = newTmp(); 
            code.push_back(t + " = " + A + " " + b->op + " " + B); 
            if(debug) cout << "[TAC] Generated: " << code.back() << endl;
            return t;
        }
        cerr<<"[TAC ERROR] Unhandled expression type\n"; 
        exit(1);
    }
    
    void genStmt(Stmt* s){
        if(auto as = dynamic_cast<AssignStmt*>(s)){
            string r = genExpr(as->e.get()); 
            code.push_back(as->name + " = " + r);
            if(debug) cout << "[TAC] Generated: " << code.back() << endl;
        } else if(auto ps = dynamic_cast<PrintStmt*>(s)){
            string r = genExpr(ps->e.get()); 
            code.push_back(string("print ") + r);
            if(debug) cout << "[TAC] Generated: " << code.back() << endl;
        } else if(auto ifs = dynamic_cast<IfStmt*>(s)){
            string c = genExpr(ifs->cond.get()); 
            string L1 = string("L") + to_string(code.size()) + "a"; 
            string L2 = string("L") + to_string(code.size()) + "b";
            code.push_back(string("ifz ") + c + " goto " + L1);
            if(debug) cout << "[TAC] Generated: " << code.back() << endl;
            genBlock(ifs->thenBlock.get()); 
            code.push_back(string("goto ") + L2);
            if(debug) cout << "[TAC] Generated: " << code.back() << endl;
            code.push_back(L1 + ":");
            if(debug) cout << "[TAC] Generated label: " << code.back() << endl;
            if(ifs->elseBlock) genBlock(ifs->elseBlock.get());
            code.push_back(L2 + ":");
            if(debug) cout << "[TAC] Generated label: " << code.back() << endl;
        } else if(auto wh = dynamic_cast<WhileStmt*>(s)){
            string L1 = string("L") + to_string(code.size()) + "a"; 
            string L2 = string("L") + to_string(code.size()) + "b";
            code.push_back(L1 + ":");
            if(debug) cout << "[TAC] Generated label: " << code.back() << endl;
            string c = genExpr(wh->cond.get()); 
            code.push_back(string("ifz ") + c + " goto " + L2);
            if(debug) cout << "[TAC] Generated: " << code.back() << endl;
            genBlock(wh->body.get()); 
            code.push_back(string("goto ") + L1);
            if(debug) cout << "[TAC] Generated: " << code.back() << endl;
            code.push_back(L2 + ":");
            if(debug) cout << "[TAC] Generated label: " << code.back() << endl;
        } else if(auto blk = dynamic_cast<BlockStmt*>(s)){
            genBlock(blk);
        } else {
            cerr<<"[TAC ERROR] Unknown statement type\n"; 
            exit(1);
        }
    }
    
    void genBlock(BlockStmt* blk){ 
        if(debug) cout << "[TAC] Generating code for block with " << blk->stmts.size() << " statements" << endl;
        for(auto &s : blk->stmts) genStmt(s.get()); 
    }
};

// ============================================================================
// PHASE 6: CODE GENERATION & EXECUTION
// ============================================================================
void runSource(const string &source, bool verbose=false, bool debug=false){
    cout << "=== MINILANG COMPILER EXECUTION ===" << endl;
    
    // PHASE 1: Lexical Analysis
    cout << "\n--- PHASE 1: LEXICAL ANALYSIS ---" << endl;
    Parser p(source, debug);
    
    // PHASE 2: Syntax Analysis  
    cout << "\n--- PHASE 2: SYNTAX ANALYSIS ---" << endl;
    auto prog = p.parseProgram();
    
    // PHASE 3: Semantic Analysis
    cout << "\n--- PHASE 3: SEMANTIC ANALYSIS ---" << endl;
    set<string> defined;
    semanticCheckBlock(prog.get(), defined);
    
    // PHASE 5: Optimization
    cout << "\n--- PHASE 5: OPTIMIZATION ---" << endl;
    foldConstantsInBlock(prog.get());
    
    // PHASE 4 & 6: Intermediate Code Generation
    cout << "\n--- PHASE 4 & 6: INTERMEDIATE CODE GENERATION ---" << endl;
    TACGen gen(debug); 
    gen.genBlock(prog.get());
    
    if(verbose){ 
        cout << "\n--- THREE ADDRESS CODE ---" << endl;
        for(auto &l: gen.code) cout << l << "\n"; 
        cout << "--- END TAC ---" << endl;
    }
    
    // PHASE 6: Execution
    cout << "\n--- PHASE 6: EXECUTION ---" << endl;
    cout << "Program Output:" << endl;
    cout << "---------------" << endl;
    map<string,long long> env;
    prog->exec(env);
    cout << "---------------" << endl;
    cout << "Execution completed!" << endl;
}

string loadFile(const string &path){ 
    ifstream in(path); 
    if(!in) { 
        cerr<<"[ERROR] Cannot open file: "<<path<<"\n"; 
        exit(1);
    } 
    string s((istreambuf_iterator<char>(in)), istreambuf_iterator<char>()); 
    return s; 
}

int main(int argc, char **argv){
    string defaultProg = R"MINI(
// compute fibonacci iteratively and print fib(10)
 a = 0;
 b = 1;
 i = 0;
 while (i < 10) {
   t = a + b;
   a = b;
   b = t;
   i = i + 1;
 }
 print(a);
)MINI";

    // Enhanced command line options
    if(argc >= 2){ 
        string arg0 = argv[1]; 
        if(arg0=="--spec" || arg0=="-spec"){ 
            cout<<MINILANG_SPEC<<"\n"; 
            return 0; 
        } 
        if(arg0=="--help" || arg0=="-h") {
            cout << "MiniLang Compiler Usage:\n";
            cout << "  ./minilang [options] [file.minilang]\n";
            cout << "Options:\n";
            cout << "  --spec, -spec    Show language specification\n";
            cout << "  -v               Verbose mode (show TAC)\n";
            cout << "  -d               Debug mode (show all phases)\n";
            cout << "  --help, -h       Show this help\n";
            return 0;
        }
    }

    string source;
    bool verbose = false;
    bool debug = false;
    
    if(argc >= 2){ 
        string arg = argv[1]; 
        if(arg=="-v") { 
            verbose=true; 
            if(argc>=3) source = loadFile(argv[2]); 
            else source = defaultProg; 
        } 
        else if(arg=="-d") {
            debug = true;
            verbose = true;
            if(argc>=3) source = loadFile(argv[2]); 
            else source = defaultProg;
        }
        else { 
            source = loadFile(arg); 
        } 
    }
    else source = defaultProg;

    runSource(source, verbose, debug);
    return 0;
}