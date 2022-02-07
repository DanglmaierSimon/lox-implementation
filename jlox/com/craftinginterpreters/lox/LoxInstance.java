package com.craftinginterpreters.lox;

import java.util.HashMap;
import java.util.Map;

public class LoxInstance {
    private final LoxClass lclass;
    private final Map<String, Object> fields = new HashMap<>();

    LoxInstance(LoxClass lclass) {
        this.lclass = lclass;
    }

    Object get(Token name) {
        if (fields.containsKey(name.lexeme)) {
            return fields.get(name.lexeme);
        }

        LoxFunction method = lclass.findMethod(name.lexeme);
        if (method != null) {
            return method.bind(this);
        }

        throw new RuntimeError(name, "Undefined property '" + name.lexeme + "'.");
    }

    @Override
    public String toString() {
        return lclass.name + " instance";
    }

    public void set(Token name, Object value) {
        fields.put(name.lexeme, value);
    }
}
