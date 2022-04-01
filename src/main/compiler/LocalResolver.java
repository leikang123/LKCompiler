package main.compiler;
import main.AST.*;
import main.entity.*;
import main.utils.ErrorHandler;
import main.exception.*;
import java.util.*;
//变量引用的消解类
public class LocalResolver extends Visitor {
    // 保存scope关系的栈
    private final LinkedList<Scope> scopeStack;
    //管理字符串常量
    private final ConstantTable constantTable;
    // 处理错误
    private final ErrorHandler errorHandler;

    public LocalResolver(ErrorHandler h) {
        this.errorHandler = h;
        this.scopeStack = new LinkedList<Scope>();
        this.constantTable = new ConstantTable();
    }

    private void resolve(StmtNode n) {
        n.accept(this);
    }

    private void resolve(ExprNode n) {
        n.accept(this);
    }

    // 消解方法实现类的启动方法
    public void resolve(AST ast) throws SemanticException {
        // 生成对象类的创建
        ToplevelScope toplevel = new ToplevelScope();
        // 把生成类的对象添加到scopeStank里面，这样栈就有一个scope对象
        scopeStack.add(toplevel);
        
        // for遍历
        for (Entity decl : ast.declarations()) {
            toplevel.declareEntity(decl);
        }
        for (Entity ent : ast.definitions()) {
            toplevel.defineEntity(ent);
        }
        resolveGvarInitializers(ast.definedVariables());
        resolveConstantValues(ast.constants());
        resolveFunctions(ast.definedFunctions());
    
        toplevel.checkReferences(errorHandler);
        if (errorHandler.errorOccured()) {
            throw new SemanticException("compile failed.");
        }

        ast.setScope(toplevel);
        ast.setConstantTable(constantTable);
    }
    
    private void resolveGvarInitializers(List<DefinedVariable> gvars) {
        for (DefinedVariable gvar : gvars) {
            if (gvar.hasInitializer()) {
                resolve(gvar.initializer());
            }
        }
    }

    private void resolveConstantValues(List<Constant> consts) {
        for (Constant c : consts) {
            resolve(c.value());
        }
    }

    
    private void resolveFunctions(List<DefinedFunction> funcs) {
        for (DefinedFunction func : funcs) {
            pushScope(func.parameters());
            resolve(func.body());
            func.setScope(popScope());
        }
    }
    
    public Void visit(BlockNode node) {
        pushScope(node.variables());
        super.visit(node);
        node.setScope(popScope());
        return null;
    }
    
    private void pushScope(List<? extends DefinedVariable> vars) {
        LocalScope scope = new LocalScope(currentScope());
        for (DefinedVariable var : vars) {
            if (scope.isDefinedLocally(var.name())) {
                error(var.location(),
                    "duplicated variable in scope: " + var.name());
            }
            else {
                scope.defineVariable(var);
            }
        }
        scopeStack.addLast(scope);
    }
    
    private LocalScope popScope() {
        return (LocalScope)scopeStack.removeLast();
    }
    
    private Scope currentScope() {
        return scopeStack.getLast();
    }
   
    public Void visit(StringLiteralNode node) {
        node.setEntry(constantTable.intern(node.value()));
        return null;
    }
    
    public Void visit(VariableNode node) {
        try {
            Entity ent = currentScope().get(node.name());
            ent.refered();
            node.setEntity(ent);
        }
        catch (SemanticException ex) {
            error(node, ex.getMessage());
        }
        return null;
    }
    

    private void error(Node node, String message) {
        errorHandler.error(node.location(), message);
    }

    private void error(Location loc, String message) {
        errorHandler.error(loc, message);
    }
}
