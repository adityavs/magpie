package com.stuffwithstuff.magpie.interpreter.builtin;

import com.stuffwithstuff.magpie.ast.Expr;
import com.stuffwithstuff.magpie.ast.pattern.Pattern;
import com.stuffwithstuff.magpie.interpreter.Callable;
import com.stuffwithstuff.magpie.interpreter.ClassObj;
import com.stuffwithstuff.magpie.interpreter.Interpreter;
import com.stuffwithstuff.magpie.interpreter.Obj;

/**
 * Built-in callable that returns the value of a named field.
 */
public class FieldGetter implements Callable {
  public FieldGetter(ClassObj classObj, String name) {
    mName = name;
    mPattern = Pattern.type(Expr.name(classObj.getName()));
  }
  
  @Override
  public Obj invoke(Interpreter interpreter, Obj arg) {
    Obj value = arg.getField(mName);
    if (value == null) return interpreter.nothing();
    return value;
  }
  
  @Override
  public Pattern getPattern() {
    return mPattern;
  }
  
  private final String mName;
  private final Pattern mPattern;
}
