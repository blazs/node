#include "qminer_nodejs.h"

///////////////////////////////
// NodeJs-GLib-TVec
v8::Persistent<v8::FunctionTemplate> TNodeJsVec::constructor;

void TNodeJsVec::Init(v8::Handle<v8::Object> target) {
   v8::HandleScope scope;

   v8::Local<v8::FunctionTemplate> tpl = v8::FunctionTemplate::New(New);
   v8::Local<v8::String> name = v8::String::NewSymbol("TVec"); 

   constructor = v8::Persistent<v8::FunctionTemplate>::New(tpl);
   // ObjectWrap uses the first internal field to store the wrapped pointer.
   constructor->InstanceTemplate()->SetInternalFieldCount(1);
   constructor->SetClassName(name);

   // Add all prototype methods, getters and setters here.
   NODE_SET_PROTOTYPE_METHOD(constructor, "at", at);
   NODE_SET_PROTOTYPE_METHOD(constructor, "sum", sum);
   NODE_SET_PROTOTYPE_METHOD(constructor, "put", put);
   NODE_SET_PROTOTYPE_METHOD(constructor, "push", push);
   NODE_SET_PROTOTYPE_METHOD(constructor, "getMaxIdx", getMaxIdx);
   NODE_SET_PROTOTYPE_METHOD(constructor, "sort", sort);
   
   tpl->InstanceTemplate()->SetAccessor(v8::String::New("length"), length);
   
   // NODE_SET_PROTOTYPE_METHOD(constructor, "length", length);
   // This has to be last, otherwise the properties won't show up on the
   // object in JavaScript.
   target->Set(name, constructor->GetFunction());
}

v8::Handle<v8::Value> TNodeJsVec::New(const v8::Arguments& Args) {
   v8::HandleScope HandleScope;
   if (!Args.IsConstructCall()) {
      return v8::ThrowException(v8::Exception::TypeError(v8::String::New(
         "Not a constructor call")));
   }
   
   TNodeJsVec* JsVec = new TNodeJsVec();
   JsVec->Wrap(Args.This());
   
   return Args.This();
}

// Returns an element at index idx=Args[0]; assert 0 <= idx < v.length() 
v8::Handle<v8::Value> TNodeJsVec::at(const v8::Arguments& Args) {
   v8::HandleScope HandleScope;
   TNodeJsVec* JsVec = ObjectWrap::Unwrap<TNodeJsVec>(Args.This());
   
   EAssertR(Args.Length() >= 1 && Args[0]->IsInt32(), "Expected integer.");
   const int Idx = Args[0]->IntegerValue();
   EAssertR(Idx >= 0 && Idx < JsVec->Vec.Len(), "Index out of bounds.");
   
   return HandleScope.Close(v8::Number::New(JsVec->Vec.GetVal(Idx).Val));
}

// Returns the sum of the vectors elements (only make sense for numeric values) 
v8::Handle<v8::Value> TNodeJsVec::sum(const v8::Arguments& Args) {
   v8::HandleScope HandleScope;
   TNodeJsVec* JsVec = ObjectWrap::Unwrap<TNodeJsVec>(Args.This());
   TInt Sum = 0;
   for (int ElN = 0; ElN < JsVec->Vec.Len(); ++ElN) {
      Sum += JsVec->Vec.GetVal(ElN);
   }
   return HandleScope.Close(v8::Number::New(Sum));
}

// put(idx, num) sets v[idx] := num 
v8::Handle<v8::Value> TNodeJsVec::put(const v8::Arguments& Args) {
   v8::HandleScope HandleScope;
   
   EAssertR(Args.Length() >= 2, "Expected two arguments.");
   EAssertR(Args[0]->IsInt32(),
      "First argument should be an integer.");
   EAssertR(Args[1]->IsNumber(),
      "Second argument should be a number.");
   
   TNodeJsVec* JsVec = ObjectWrap::Unwrap<TNodeJsVec>(Args.This());
   const int Idx = Args[0]->IntegerValue();
   
   EAssertR(Idx >= 0 && Idx < JsVec->Vec.Len(), "Index out of bounds");
   
   JsVec->Vec[Idx] = Args[1]->NumberValue();
   
   return v8::Boolean::New(true);
}

// Appends an element to the vector 
v8::Handle<v8::Value> TNodeJsVec::push(const v8::Arguments& Args) {
   v8::HandleScope HandleScope;
   
   TNodeJsVec* JsVec = ObjectWrap::Unwrap<TNodeJsVec>(Args.This());
   
   const double Val = Args[0]->ToNumber()->Value();
   
   JsVec->Vec.Add(Val);
   
   return HandleScope.Close(v8::Boolean::New(true));
}

// Returns i = arg max_i v[i] for a vector v 
v8::Handle<v8::Value> TNodeJsVec::getMaxIdx(const v8::Arguments& Args) {
   v8::HandleScope HandleScope;
   TNodeJsVec* JsVec = ObjectWrap::Unwrap<TNodeJsVec>(Args.This());
   EAssertR(!JsVec->Vec.Empty(), "getMaxIdx is undefined for empty vector.");
   
   double MxVal = JsVec->Vec.GetVal(0);
   int MxIdx = 0;
   for (int ElN = 0; ElN < JsVec->Vec.Len(); ++ElN) {
      const double CrrVal = JsVec->Vec.GetVal(ElN);
      if (CrrVal > MxVal) { MxIdx = ElN; MxVal = CrrVal; }
   }
   
   return HandleScope.Close(v8::Integer::New(MxIdx));
}

// TODO: How to create a new instance of TNodeJsVec on the C++ side? 
v8::Handle<v8::Value> TNodeJsVec::sort(const v8::Arguments& Args) {
   v8::HandleScope HandleScope;
   TNodeJsVec* JsVec = ObjectWrap::Unwrap<TNodeJsVec>(Args.This());
   EAssertR(Args.Length() == 0 || Args[0]->IsBoolean(), "Expected boolean");
   
   const bool Asc = Args.Length() > 0 && Args[0]->BooleanValue();
   
   TNodeJsVec* JsResV = new TNodeJsVec();
   
   JsResV->Wrap(Args.This());
   JsResV->Vec.AddV(JsVec->Vec);
   JsResV->Vec.Sort(Asc);
   
   return Args.This();
}

// Returns the size of the vector 
v8::Handle<v8::Value> TNodeJsVec::length(v8::Local<v8::String> Properties, const v8::AccessorInfo& Info) {
	v8::HandleScope HandleScope;
	
	v8::Local<v8::Object> Self = Info.Holder();
	TNodeJsVec* JsVec = ObjectWrap::Unwrap<TNodeJsVec>(Self);
	
	return HandleScope.Close(v8::Integer::New(JsVec->Vec.Len()));
}

///////////////////////////////
// Register functions, etc.  
void init(v8::Handle<v8::Object> target) {
   // target->Set(v8::String::NewSymbol("Test"),
   //   v8::FunctionTemplate::New(Test)->GetFunction());
   // TNodeJsLinAlg::Init(target);
   TNodeJsVec::Init(target);
}

NODE_MODULE(qminer, init)

