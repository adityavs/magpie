package com.stuffwithstuff.magpie.interpreter;

import java.util.*;

import com.stuffwithstuff.magpie.ast.Expr;
import com.stuffwithstuff.magpie.ast.Field;

/**
 * A runtime object representing a class.
 */
public class ClassObj extends Obj {
  /**
   * Creates a new class object.
   * 
   * @param classClass  The class of this class.
   * @param name        The name of the class.
   */
  public ClassObj(ClassObj classClass, String name, List<ClassObj> parents,
      Map<String, Field> fields) {
    super(classClass);
    mName = name;

    if (parents != null) {
      mParents = parents;
    } else {
      mParents = new ArrayList<ClassObj>();
    }
    
    mFields = fields;
  }
  
  public String getName() { return mName; }
  public List<ClassObj> getParents() { return mParents; }
  public Map<String, Field> getFieldDefinitions() { return mFields; }
  
  /**
   * Gets whether or not this class is a subclass (or same class) as the given
   * parent.
   */
  public boolean isSubclassOf(ClassObj parent) {
    if (this == parent) return true;
    
    for (ClassObj myParent : mParents) {
      if (myParent.isSubclassOf(parent)) return true;
    }
    
    return false;
  }

  public void declareField(String name, Expr type) {
    // Declare the field.
    mFields.put(name, new Field(null, type));
    
    // Add a getter and setter.
    /*
    mMembers.defineGetter(name, new FieldGetter(name));
    mMembers.defineSetter(name, new FieldSetter(name));
    */
  }
  
  @Override
  public String toString() {
    return mName;
  }
  
  private final String mName;
  
  private final List<ClassObj> mParents;
  private final Map<String, Field> mFields;
}
