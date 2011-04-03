package com.stuffwithstuff.magpie.interpreter.builtin;

import com.stuffwithstuff.magpie.ast.Expr;
import com.stuffwithstuff.magpie.ast.Field;
import com.stuffwithstuff.magpie.ast.pattern.Pattern;
import com.stuffwithstuff.magpie.interpreter.Callable;
import com.stuffwithstuff.magpie.interpreter.ClassObj;
import com.stuffwithstuff.magpie.interpreter.Interpreter;
import com.stuffwithstuff.magpie.interpreter.Obj;

/**
 * Built-in callable that assigns a value to a named field.
 */
public class FieldSetter implements Callable {
  public FieldSetter(ClassObj classObj, String name, Field field) {
    mName = name;
    mPattern = Pattern.tuple(
        Pattern.type(Expr.name(classObj.getName())),
        field.getPattern());
  }
  
  @Override
  public Obj invoke(Interpreter interpreter, Obj arg) {
    arg.getTupleField(0).setField(mName, arg.getTupleField(1));
    return arg.getTupleField(1);
  }

  @Override
  public Pattern getPattern() {
    return mPattern;
  }
  
  private final String mName;
  private final Pattern mPattern;
}
