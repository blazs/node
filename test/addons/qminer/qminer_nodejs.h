#ifndef QMINER_NODEJS
#define QMINER_NODEJS

#define BUILDING_NODE_EXTENSION

#include <node.h>
#include "base.h"

#define JsDeclareProperty(Function) \
	static v8::Handle<v8::Value> Function(v8::Local<v8::String> Properties, const v8::AccessorInfo& Info);

#define JsDeclareFunction(Function) static v8::Handle<v8::Value> Function(const v8::Arguments& Args);

#define QmAssert(Cond) \
  EAssert(Cond)

#define QmAssertR(Cond, MsgStr) \
  EAssertR(Cond, TStr(MsgStr).CStr())

///////////////////////////////
/// JavaScript Utility Function
class TNodeJsUtil {
private:
	/// Marks when internals already initialized
	static TBool InitP;
	/// Must be called at the beginning to initialize
	static void Init();

	// JSON parsing
	static v8::Persistent<v8::Function> JsonParser;
	static v8::Persistent<v8::Function> JsonString;

    /// Count of current number of objects of each type
    static TStrH ObjNameH;
    /// Number of changes (new/delete) so far
    static TInt ObjCount;    
    /// How often to print the statistics 
    static TInt ObjCountRate;
public:
    /// Add new object to the count table
	static void AddObj(const TStr& ObjName);
    /// Remove existing object from the count table
	static void DelObj(const TStr& ObjName);
    /// Print statistics each so often
	static void CountObj();
    /// Get current statistics
    static TStrIntPrV GetObjStat();
    /// Set rate at which statistics is printed
    static void SetObjStatRate(const int& _ObjCountRate);
   
	// Parses PJsonVal to V8::Value
	static v8::Handle<v8::Value> ParseJson(const PJsonVal& JsonVal);
	// Parses TStr as json to V8::Value
	static v8::Handle<v8::Value> ParseJson(const TStr& JsonStr);
	// Parses v8 JSon to V8 String
	static v8::Handle<v8::Value> V8JsonToV8Str(v8::Handle<v8::Value> Json);
	// Parses V8 Json to TStr
	static TStr V8JsonToStr(v8::Handle<v8::Value> Json);

	/// Takes GLib's TTm and returns V8's Date 
	static v8::Handle<v8::Value> GetV8Date(const TTm& Tm);
	/// Takes V8's Date and returns GLib's TTm 
	static TTm GetGlibDate(v8::Handle<v8::Value> Date);	
	/// Returns V8's current date 
	static v8::Handle<v8::Value> GetCurrV8Date();

    /// TStrV -> v8 string array
    static v8::Handle<v8::Value> GetStrArr(const TStrV& StrV);
};

///////////////////////////////
// QMiner-JavaScript-Object-Utility
template <class TNodeJsObj>
class TNodeJsObjUtil {
public:
   /// Extracts pointer to TJsObj from v8 Arguments
	static TNodeJsObj* GetSelf(const v8::Arguments& Args) {
		v8::HandleScope HandleScope;
		v8::Handle<v8::Object> Self = Args.Holder();
		v8::Local<v8::External> WrappedObject = v8::Local<v8::External>::Cast(Self->GetInternalField(0));
		TNodeJsObj* JsObj = static_cast<TNodeJsObj*>(WrappedObject->Value());
		return JsObj;
	}

	/// Extracts pointer to TJsObj from v8 AccessorInfo object
	static TNodeJsObj* GetSelf(const v8::AccessorInfo& Info) {
		v8::HandleScope HandleScope;
		v8::Local<v8::Object> Self = Info.Holder();
		v8::Local<v8::External> WrappedObject = v8::Local<v8::External>::Cast(Self->GetInternalField(0));
		TNodeJsObj* JsObj = static_cast<TNodeJsObj*>(WrappedObject->Value());
		return JsObj;
	}

	static TNodeJsObj* GetSelf(const v8::Handle<v8::Object> Obj) {
		v8::HandleScope HandleScope;
		v8::Local<v8::External> WrappedObject = v8::Local<v8::External>::Cast(Obj->GetInternalField(0));
		TNodeJsObj* JsObj = static_cast<TNodeJsObj*>(WrappedObject->Value());
		return JsObj;
	}
	
	/// Transform V8 string to TStr
	static TStr GetStr(const v8::Local<v8::String>& V8Str) {
		v8::HandleScope HandleScope;
		v8::String::Utf8Value Utf8(V8Str);
		return TStr(*Utf8);
	}

	/// Extract name of the function called by the script
	static TStr GetFunNm(const v8::Arguments& Args) {
		v8::Handle<v8::Value> Val = Args.Callee()->GetName();
		v8::String::Utf8Value Utf8(Val);
		return TStr(*Utf8);
	}

	/// Check if argument ArgN exists
	static bool IsArg(const v8::Arguments& Args, const int& ArgN) {
		return (Args.Length() > ArgN);
	}

	/// Checks if argument ArgN is of type string
	static bool IsArgStr(const v8::Arguments& Args, const int& ArgN) {
		v8::HandleScope HandleScope;
		QmAssertR(Args.Length() > ArgN, TStr::Fmt("Missing argument %d", ArgN));
		v8::Handle<v8::Value> Val = Args[ArgN];
		return Val->IsString();
	}

	/// Extract argument ArgN as TStr
	static TStr GetArgStr(const v8::Arguments& Args, const int& ArgN) {
		v8::HandleScope HandleScope;
		QmAssertR(Args.Length() > ArgN, TStr::Fmt("Missing argument %d", ArgN));
		v8::Handle<v8::Value> Val = Args[ArgN];
		QmAssertR(Val->IsString(), TStr::Fmt("Argument %d expected to be string", ArgN));
		v8::String::Utf8Value Utf8(Val);
		return TStr(*Utf8);
	}

	/// Extract argument ArgN as TStr, and use DefVal in case when not present
	static TStr GetArgStr(const v8::Arguments& Args, const int& ArgN, const TStr& DefVal) {
		v8::HandleScope HandleScope;
		if (Args.Length() > ArgN) {
			v8::Handle<v8::Value> Val = Args[ArgN];
			QmAssertR(Val->IsString(), TStr::Fmt("Argument %d expected to be string", ArgN));
			v8::String::Utf8Value Utf8(Val);
			return TStr(*Utf8);
		}
		return DefVal;
	}

	/// Extract argument ArgN property as string
	static TStr GetArgStr(const v8::Arguments& Args, const int& ArgN, const TStr& Property, const TStr& DefVal) {
		v8::HandleScope HandleScope;
		if (Args.Length() > ArgN) {
			if (Args[ArgN]->IsObject() && Args[ArgN]->ToObject()->Has(v8::String::New(Property.CStr()))) {
				v8::Handle<v8::Value> Val = Args[ArgN]->ToObject()->Get(v8::String::New(Property.CStr()));
				QmAssertR(Val->IsString(), TStr::Fmt("Argument %d, property %s expected to be string", ArgN, Property.CStr()));
				v8::String::Utf8Value Utf8(Val);
				return TStr(*Utf8);
			}
		}
		return DefVal;
	}
   
	/// Check if argument ArgN is of type boolean
	static bool IsArgBool(const v8::Arguments& Args, const int& ArgN) {
		v8::HandleScope HandleScope;
		QmAssertR(Args.Length() > ArgN, TStr::Fmt("Missing argument %d", ArgN));
		v8::Handle<v8::Value> Val = Args[ArgN];
		return Val->IsBoolean();
	}

	/// Extract argument ArgN as boolean
	static bool GetArgBool(const v8::Arguments& Args, const int& ArgN) {
		v8::HandleScope HandleScope;
		QmAssertR(Args.Length() > ArgN, TStr::Fmt("Missing argument %d", ArgN));
		v8::Handle<v8::Value> Val = Args[ArgN];
		QmAssertR(Val->IsBoolean(), TStr::Fmt("Argument %d expected to be boolean", ArgN));
		return static_cast<bool>(Val->BooleanValue());
	}

	/// Extract argument ArgN as boolean, and use DefVal in case when not present
	static bool GetArgBool(const v8::Arguments& Args, const int& ArgN, const bool& DefVal) {
		v8::HandleScope HandleScope;
		if (Args.Length() > ArgN) {
			v8::Handle<v8::Value> Val = Args[ArgN];
			QmAssertR(Val->IsBoolean(), TStr::Fmt("Argument %d expected to be boolean", ArgN));
			return static_cast<bool>(Val->BooleanValue());
		}
		return DefVal;
	}
	
	/// Extract argument ArgN property as bool
	static bool GetArgBool(const v8::Arguments& Args, const int& ArgN, const TStr& Property, const bool& DefVal) {
		v8::HandleScope HandleScope;
		if (Args.Length() > ArgN) {			
			if (Args[ArgN]->IsObject() && Args[ArgN]->ToObject()->Has(v8::String::New(Property.CStr()))) {
				v8::Handle<v8::Value> Val = Args[ArgN]->ToObject()->Get(v8::String::New(Property.CStr()));
				 QmAssertR(Val->IsBoolean(), TStr::Fmt("Argument %d, property %s expected to be boolean", ArgN, Property.CStr()));
				 return static_cast<bool>(Val->BooleanValue());
			}
		}
		return DefVal;
	}
        
	/// Check if argument ArgN is of type integer
	static bool IsArgInt32(const v8::Arguments& Args, const int& ArgN) {
		v8::HandleScope HandleScope;
		QmAssertR(Args.Length() > ArgN, TStr::Fmt("Missing argument %d", ArgN));
		v8::Handle<v8::Value> Val = Args[ArgN];
		return Val->IsInt32();
	}

	/// Extract argument ArgN as integer
	static int GetArgInt32(const v8::Arguments& Args, const int& ArgN) {
		v8::HandleScope HandleScope;
		QmAssertR(Args.Length() > ArgN, TStr::Fmt("Missing argument %d", ArgN));
		v8::Handle<v8::Value> Val = Args[ArgN];
		QmAssertR(Val->IsInt32(), TStr::Fmt("Argument %d expected to be int32", ArgN));
		return static_cast<int>(Val->Int32Value());
	}

	/// Extract argument ArgN as integer, and use DefVal in case when not present
	static int GetArgInt32(const v8::Arguments& Args, const int& ArgN, const int& DefVal) {
		v8::HandleScope HandleScope;
		if (Args.Length() > ArgN) {
			v8::Handle<v8::Value> Val = Args[ArgN];
			QmAssertR(Val->IsInt32(), TStr::Fmt("Argument %d expected to be int32", ArgN));
			return static_cast<int>(Val->Int32Value());
		}
		return DefVal;
	}
	/// Extract argument ArgN property as int
	static int GetArgInt32(const v8::Arguments& Args, const int& ArgN, const TStr& Property, const int& DefVal) {
		v8::HandleScope HandleScope;
		if (Args.Length() > ArgN) {			
			if (Args[ArgN]->IsObject() && Args[ArgN]->ToObject()->Has(v8::String::New(Property.CStr()))) {
				v8::Handle<v8::Value> Val = Args[ArgN]->ToObject()->Get(v8::String::New(Property.CStr()));
				 QmAssertR(Val->IsInt32(), TStr::Fmt("Argument %d, property %s expected to be int32", ArgN, Property.CStr()));
				 return Val->ToNumber()->Int32Value();
			}
		}
		return DefVal;
	}
     
   	/// Check if argument ArgN is of type double
	static bool IsArgFlt(const v8::Arguments& Args, const int& ArgN) {
		v8::HandleScope HandleScope;
		QmAssertR(Args.Length() > ArgN, TStr::Fmt("Missing argument %d", ArgN));
		v8::Handle<v8::Value> Val = Args[ArgN];
		return Val->IsNumber();
	}

	/// Extract argument ArgN as double
	static double GetArgFlt(const v8::Arguments& Args, const int& ArgN) {
		v8::HandleScope HandleScope;
		QmAssertR(Args.Length() > ArgN, TStr::Fmt("Missing argument %d", ArgN));
		v8::Handle<v8::Value> Val = Args[ArgN];
		QmAssertR(Val->IsNumber(), TStr::Fmt("Argument %d expected to be double", ArgN));
		return static_cast<double>(Val->NumberValue());
	}

	/// Extract argument ArgN as a js object
	static TNodeJsObj* GetArgObj(const v8::Arguments& Args, const int& ArgN) {
		v8::Handle<v8::Value> Val = Args[ArgN];
    	v8::Handle<v8::Object> Data = v8::Handle<v8::Object>::Cast(Val);	
		v8::Local<v8::External> WrappedObject = v8::Local<v8::External>::Cast(Data->GetInternalField(0));
		return static_cast<TNodeJsObj*>(WrappedObject->Value());		
	}

	/// Extract argument ArgN as double, and use DefVal in case when not present
	static double GetArgFlt(const v8::Arguments& Args, const int& ArgN, const double& DefVal) {
		v8::HandleScope HandleScope;
		if (Args.Length() > ArgN) {
			v8::Handle<v8::Value> Val = Args[ArgN];
			QmAssertR(Val->IsNumber(), TStr::Fmt("Argument %d expected to be double", ArgN));
			return static_cast<double>(Val->NumberValue());
		}
		return DefVal;
	}
    
   	/// Check if argument ArgN is of type json
	static bool IsArgJson(const v8::Arguments& Args, const int& ArgN) {
		v8::HandleScope HandleScope;
		QmAssertR(Args.Length() > ArgN, TStr::Fmt("Missing argument %d", ArgN));
		v8::Handle<v8::Value> Val = Args[ArgN];
		return Val->IsObject();
	}    

	/// Extract Val as JSon object, and serialize it to TStr
	static TStr GetValJsonStr(const v8::Handle<v8::Value> Val) {
		v8::HandleScope HandleScope;
		QmAssertR(Val->IsObject(), "Val expected to be object");
		TStr JsonStr = TNodeJsUtil::V8JsonToStr(Val);
		return JsonStr;
	}

	/// Extract argument ArgN as JSon object, and serialize it to TStr
	static TStr GetArgJsonStr(const v8::Arguments& Args, const int& ArgN) {
		v8::HandleScope HandleScope;
		QmAssertR(Args.Length() > ArgN, TStr::Fmt("Missing argument %d", ArgN));
		v8::Handle<v8::Value> Val = Args[ArgN];
		QmAssertR(Val->IsObject(), TStr::Fmt("Argument %d expected to be json", ArgN));
		TStr JsonStr = TNodeJsUtil::V8JsonToStr(Val);
		return JsonStr;
	}

	static TStr GetArgJsonStr(const v8::Arguments& Args, const int& ArgN, const TStr& Property) {
		v8::HandleScope HandleScope;
		QmAssertR(Args.Length() > ArgN, TStr::Fmt("TJsObjUtil::GetArgJsonStr : Missing argument %d", ArgN));
		QmAssertR(Args[ArgN]->IsObject() &&
			Args[ArgN]->ToObject()->Has(v8::String::New(Property.CStr())),
			TStr::Fmt("TJsObjUtil::GetArgJsonStr : Argument %d must be an object with property %s", ArgN, Property.CStr()));

		v8::Handle<v8::Value> Val = Args[ArgN]->ToObject()->Get(v8::String::New(Property.CStr()));
		QmAssertR(Val->IsObject(), TStr::Fmt("TJsObjUtil::GetArgJsonStr : Args[%d].%s expected to be json", ArgN, Property.CStr()));
		TStr JsonStr = TNodeJsUtil::V8JsonToStr(Val);
		return JsonStr;
	}

	/// Extract argument ArgN as JSon object, and transform it to PJsonVal
	static PJsonVal GetArgJson(const v8::Arguments& Args, const int& ArgN) {
		TStr JsonStr = GetArgJsonStr(Args, ArgN);
		PJsonVal Val = TJsonVal::GetValFromStr(JsonStr);
		if (!Val->IsDef()) { FailR(TStr("Error parsing '" + JsonStr + "'.").CStr()); }
		return Val;
	}

	/// Extract Val as JSon object, and transform it to PJsonVal
	static PJsonVal GetValJson(const v8::Handle<v8::Value> Val) {
		TStr JsonStr = GetValJsonStr(Val);
		PJsonVal JsonVal = TJsonVal::GetValFromStr(JsonStr);
		if (!JsonVal->IsDef()) { FailR(TStr("Error parsing '" + JsonStr + "'.").CStr()); }
		return JsonVal;
	}

	/// Extract argument ArgN property as json
	static PJsonVal GetArgJson(const v8::Arguments& Args, const int& ArgN, const TStr& Property) {
		v8::HandleScope HandleScope;
		QmAssertR(Args.Length() > ArgN, TStr::Fmt("TJsObjUtil::GetArgJson : Missing argument %d", ArgN));
		QmAssertR(Args[ArgN]->IsObject(), TStr::Fmt("TJsObjUtil::GetArgJson : Argument %d must be an object", ArgN));
		QmAssertR(Args[ArgN]->ToObject()->Has(v8::String::New(Property.CStr())), TStr::Fmt("TJsObjUtil::GetArgJson : Argument %d must have property %s", ArgN, Property.CStr()));
		TStr JsonStr = GetArgJsonStr(Args, ArgN, Property);
		PJsonVal Val = TJsonVal::GetValFromStr(JsonStr);
		if (!Val->IsDef()) { FailR(TStr("TJsObjUtil::GetArgJson : Error parsing '" + JsonStr + "'.").CStr()); }
		return Val;
	}

	static bool IsArgFun(const v8::Arguments& Args, const int& ArgN) {
		v8::HandleScope HandleScope;
		QmAssertR(Args.Length() > ArgN, TStr::Fmt("Missing argument %d", ArgN));
		v8::Handle<v8::Value> Val = Args[ArgN];
		return Val->IsFunction();
	}

	/// Extract argument ArgN as JavaScript function
	static v8::Handle<v8::Function> GetArgFun(const v8::Arguments& Args, const int& ArgN) {
		v8::HandleScope HandleScope;
		QmAssertR(Args.Length() > ArgN, TStr::Fmt("Missing argument %d", ArgN));
		v8::Handle<v8::Value> Val = Args[ArgN];
		QmAssertR(Val->IsFunction(), TStr::Fmt("Argument %d expected to be function", ArgN));
		return HandleScope.Close(v8::Handle<v8::Function>::Cast(Val));
	}
	
	/// Extract argument ArgN as persistent JavaScript function
	static v8::Persistent<v8::Function> GetArgFunPer(const v8::Arguments& Args, const int& ArgN) {
		v8::HandleScope HandleScope;
		QmAssertR(Args.Length() > ArgN, TStr::Fmt("Missing argument %d", ArgN));
		v8::Handle<v8::Value> Val = Args[ArgN];
		QmAssertR(Val->IsFunction(), TStr::Fmt("Argument %d expected to be function", ArgN));
		return v8::Persistent<v8::Function>::New(v8::Handle<v8::Function>::Cast(Val));
	}

	/// Extract argument ArgN property as persistent Javascript function
	static v8::Persistent<v8::Function> GetArgFunPer(const v8::Arguments& Args, const int& ArgN, const TStr& Property) {
		v8::HandleScope HandleScope;
		QmAssertR(Args.Length() > ArgN, TStr::Fmt("TJsObjUtil::GetArgFunPer : Missing argument %d", ArgN));
		QmAssertR(Args[ArgN]->IsObject() &&
			Args[ArgN]->ToObject()->Has(v8::String::New(Property.CStr())),
			TStr::Fmt("TJsObjUtil::GetArgFunPer : Argument %d must be an object with property %s", ArgN, Property.CStr()));
		v8::Handle<v8::Value> Val = Args[ArgN]->ToObject()->Get(v8::String::New(Property.CStr()));
		QmAssertR(Val->IsFunction(), TStr::Fmt("TJsObjUtil::GetArgFunPer Argument[%d].%s expected to be function", ArgN, Property.CStr()));
		//return HandleScope.Close(v8::Handle<v8::Function>::Cast(Val));
		return v8::Persistent<v8::Function>::New(v8::Handle<v8::Function>::Cast(Val));
	}
};

///////////////////////////////
// NodeJs-GLib-TVec
class TNodeJsVec : public node::ObjectWrap {
   static v8::Persistent<v8::FunctionTemplate> constructor;
   typedef TNodeJsObjUtil<TNodeJsVec> TNodeJsVecObjUtil;
public: // So we can register the class 
   static void Init(v8::Handle<v8::Object> target);
public:
   /*
   static v8::Persistent<v8::Object> New(TWPt<TScript> Js) {
      v8::Persistent<v8::Object> obj = TJsVecUtil::New(new TJsVec(Js));
      v8::Handle<v8::String> key = v8::String::New("class");
      v8::Handle<v8::String> value = v8::String::New(TAux::ClassId.CStr());
      obj->SetHiddenValue(key, value);      
      return  obj;
   }
   static v8::Persistent<v8::Object> New(TWPt<TScript> Js, const TValV& _Vec) {
      v8::Persistent<v8::Object> obj = New(Js);
      TJsVec::SetVec(obj, _Vec);      
      return  obj;
   }
   static TValV& GetVec(const v8::Handle<v8::Object> Obj) {
      return TJsVecUtil::GetSelf(Obj)->Vec;
   }
   static void SetVec(const v8::Handle<v8::Object> Obj, const TValV& _Vec) {
      TJsVecUtil::GetSelf(Obj)->Vec = _Vec;
   }
   */
   
   //#- `val = new qm.TVec()` -- returns new vector 
   JsDeclareFunction(New);
   //#- `num = vec.at(idx)` -- gets the value `num` of vector `vec` at index `idx`  (0-based indexing)
	//#- `num = intVec.at(idx)` -- gets the value `num` of integer vector `intVec` at index `idx`  (0-based indexing)
	JsDeclareFunction(at);
	//#- `vec2 = vec.subVec(intVec)` -- gets the subvector based on an index vector `intVec` (indices can repeat, 0-based indexing)
	//#- `intVec2 = intVec.subVec(intVec)` -- gets the subvector based on an index vector `intVec` (indices can repeat, 0-based indexing)
	//// JsDeclareFunction(subVec);
	//#- `num = vec[idx]; vec[idx] = num` -- get value `num` at index `idx`, set value at index `idx` to `num` of vector `vec`(0-based indexing)
	//// JsDeclGetSetIndexedProperty(indexGet, indexSet);
	//#- `vec = vec.put(idx, num)` -- set value of vector `vec` at index `idx` to `num` (0-based indexing). Returns self.
	//#- `intVec = intVec.put(idx, num)` -- set value of integer vector `intVec` at index `idx` to `num` (0-based indexing). Returns self.
	JsDeclareFunction(put);	
	//#- `len = vec.push(num)` -- append value `num` to vector `vec`. Returns `len` - the length  of the modified array
	//#- `len = intVec.push(num)` -- append value `num` to integer vector `intVec`. Returns `len` - the length  of the modified array
	JsDeclareFunction(push);
	//#- `len = vec.unshift(num)` -- insert value `num` to the begining of vector `vec`. Returns the length of the modified array.
	//#- `len = intVec.unshift(num)` -- insert value `num` to the begining of integer vector `intVec`. Returns the length of the modified array.
	////JsDeclareFunction(unshift);
	//#- `len = vec.pushV(vec2)` -- append vector `vec2` to vector `vec`.
	//#- `len = intVec.pushV(intVec2)` -- append integer vector `intVec2` to integer vector `intVec`.
	////JsDeclareTemplatedFunction(pushV);
	//#- `num = vec.sum()` -- return `num`: the sum of elements of vector `vec`
	//#- `num = intVec.sum()` -- return `num`: the sum of elements of integer vector `intVec`
	JsDeclareFunction(sum);
	//#- `idx = vec.getMaxIdx()` -- returns the integer index `idx` of the maximal element in vector `vec`
	//#- `idx = intVec.getMaxIdx()` -- returns the integer index `idx` of the maximal element in integer vector `vec`
	JsDeclareFunction(getMaxIdx);
	//#- `vec2 = vec.sort(asc)` -- `vec2` is a sorted copy of `vec`. `asc=true` sorts in ascending order (equivalent `sort()`), `asc`=false sorts in descending order
	//#- `intVec2 = intVec.sort(asc)` -- integer vector `intVec2` is a sorted copy of integer vector `intVec`. `asc=true` sorts in ascending order (equivalent `sort()`), `asc`=false sorts in descending order
	JsDeclareFunction(sort);
	//#- `sortRes = vec.sortPerm(asc)` -- returns a sorted copy of the vector in `sortRes.vec` and the permutation `sortRes.perm`. `asc=true` sorts in ascending order (equivalent `sortPerm()`), `asc`=false sorts in descending order.
	////JsDeclareTemplatedFunction(sortPerm);	
	//#- `vec = vec.shuffle()` -- shuffels the vector `vec` (inplace operation). Returns self.
	JsDeclareFunction(shuffle);
	//#- `vec = vec.trunc(num)` -- truncates the vector `vec` to lenght 'num' (inplace operation). Returns self.
	JsDeclareFunction(trunc);
	//#- `mat = vec.outer(vec2)` -- the dense matrix `mat` is a rank-1 matrix obtained by multiplying `vec * vec2^T`. Implemented for dense float vectors only. 
	////JsDeclareTemplatedFunction(outer);
	//#- `num = vec.inner(vec2)` -- `num` is the standard dot product between vectors `vec` and `vec2`. Implemented for dense float vectors only.
	////JsDeclareTemplatedFunction(inner);
	//#- `vec3 = vec.plus(vec2)` --`vec3` is the sum of vectors `vec` and `vec2`. Implemented for dense float vectors only.
	////JsDeclareTemplatedFunction(plus);
	//#- `vec3 = vec.minus(vec2)` --`vec3` is the difference of vectors `vec` and `vec2`. Implemented for dense float vectors only.
	////JsDeclareTemplatedFunction(minus);
	//#- `vec2 = vec.multiply(num)` --`vec2` is a vector obtained by multiplying vector `vec` with a scalar (number) `num`. Implemented for dense float vectors only.
	////JsDeclareTemplatedFunction(multiply);
	//#- `vec = vec.normalize()` -- normalizes the vector `vec` (inplace operation). Implemented for dense float vectors only. Returns self.
	////JsDeclareTemplatedFunction(normalize);
	//#- `len = vec.length` -- integer `len` is the length of vector `vec`
	//#- `len = intVec.length` -- integer `len` is the length of integer vector `vec`
	JsDeclareProperty(length);
	//#- `vec = vec.print()` -- print vector in console. Returns self.
	//#- `intVec = intVec.print()` -- print integer vector in console. Returns self.
	////JsDeclareFunction(print);
	//#- `mat = vec.diag()` -- `mat` is a diagonal dense matrix whose diagonal equals `vec`. Implemented for dense float vectors only.
	////JsDeclareTemplatedFunction(diag);
	//#- `spMat = vec.spDiag()` -- `spMat` is a diagonal sparse matrix whose diagonal equals `vec`. Implemented for dense float vectors only.
	////JsDeclareTemplatedFunction(spDiag);
	//#- `num = vec.norm()` -- `num` is the Euclidean norm of `vec`. Implemented for dense float vectors only.
	////JsDeclareTemplatedFunction(norm);
	//#- `spVec = vec.sparse()` -- `spVec` is a sparse vector representation of dense vector `vec`. Implemented for dense float vectors only.
	////JsDeclareTemplatedFunction(sparse);
	//#- `mat = vec.toMat()` -- `mat` is a matrix with a single column that is equal to dense vector `vec`.
	//#- `mat = intVec.toMat()` -- `mat` is a matrix with a single column that is equal to dense integer vector `intVec`.
	////JsDeclareTemplatedFunction(toMat);
private:
   typedef TVec<TFlt> TValV;
   TFltV Vec;
};

#endif

