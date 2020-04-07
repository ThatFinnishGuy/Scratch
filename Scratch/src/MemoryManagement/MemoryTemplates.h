#pragma once
/*
Test is a class taking 3 ints in the constructor
Test* test = ME_NEW(Test, allocator)(0, 1, 2);

the preprocessor expands the above into:
Test* test = new (allocator.Allocate(sizeof(Test), "main.cpp", 'line where allocation was made')) Test(0, 1, 2);
*/
#define MM_NEW(type, allocator)    new (allocator.allocate(sizeof(type), __FILE__, __LINE__)) type
#define MM_DELETE(object, allocator)    Delete(object, allocator)

/*
Some macro magic necessary to deduce the sizeof(T) when using our implementation of new[]. Notice that T* NewArray(ALLOCATOR& allocator, size_t N, const char* file, int line)
does not take in sizeof(T) at any point, thus it does not know the amount of memory that it needs to implement for the number of instances N. We use TypeAndCount template to
determine the type and number of instances of the values we pass.
*/

#define MM_NEW_ARRAY(type, allocator)    NewArray<TypeAndCount<type>::Type>(allocator, TypeAndCount<type>::Count, __FILE__, __LINE__, IntToType<IsPOD<TypeAndCount<type>::Type>::Value>())
#define MM_DELETE_ARRAY(object, allocator)    DeleteArray(object, allocator)




//HELPER TEMPLATES *********

/*
The base template TypeAndCount simply takes a single template argument, and is completely empty.
But by providing a partial template specialization for types taking the form T[N], we can define both the type and extract N at compile time.
*/
template <class T>
struct TypeAndCount
{
};

template <class T, size_t N>
struct TypeAndCount<T[N]>
{
	typedef T Type;
	static const size_t Count = N;
};
//------------------------------

//Type-based dispatching: This is used to satisfy the function overloads for our NewArray and DeleteArray templates (as function overloads in C++ only work based on types, not values).. cool stuff
template <bool I>
struct IntToType
{
};

typedef IntToType<false> NonPODType;
typedef IntToType<true> PODType;

//This template checks to see whether the passed value is POD.
template <typename T>
struct IsPOD
{
	static const bool Value = std::is_pod<T>::value;
};
//-------------------------------
//*******************


//This is relevant as the placement-delete doesn't call the destructor (where the delete operator does this automatically).
template <typename T, class ALLOCATOR>
void Delete(T* object, ALLOCATOR& allocator)
{
	// call the destructor first...
	object->~T();

	// ...and free the associated memory
	allocator.Clear(object);
}
//------------------------------

//--New--

//Implementation for NonPOD-types.
template <typename T, class ALLOCATOR>
T* NewArray(ALLOCATOR& allocator, size_t N, const char* file, int line, NonPODType)
{
	union
	{
		void* as_void;
		size_t* as_size_t;
		T* as_T;
	};

	as_void = allocator.allocate(sizeof(T)*N + sizeof(size_t), file, line);

	// store number of instances in first size_t bytes
	*as_size_t++ = N;

	// construct instances using placement new
	const T* const onePastLast = as_T + N;
	while (as_T < onePastLast)
		new (as_T++) T;

	// hand user the pointer to the first instance
	return (as_T - N);
}

//Implementation for POD-types.
template <typename T, class ALLOCATOR>
T* NewArray(ALLOCATOR& allocator, size_t N, const char* file, int line, PODType)
{
	return static_cast<T*>(allocator.Allocate(sizeof(T)*N, file, line));
}

//--Delete--

//This function is called by the macro, it then points to either the NonPOD or POD -functions. The reasoning why the POD check is done here and not
//in the macro (as seen with NewArray), is that in the macro "ME_DELETE_ARRAY(object, allocator)" object is a value, not a type. We don’t want the user 
//to explicitly specify the type in the macro either, because the compiler already knows it.
template <typename T, class ALLOCATOR>
void DeleteArray(T* ptr, ALLOCATOR& allocator)
{
	DeleteArray(ptr, allocator, IntToType<IsPOD<T>::Value>());
}

//Implementation for NonPOD-types.
template <typename T, class ALLOCATOR>
void DeleteArray(T* ptr, ALLOCATOR& allocator, NonPODType)
{
	union
	{
		size_t* as_size_t;
		T* as_T;
	};

	// user pointer points to first instance...
	as_T = ptr;

	// ...so go back size_t bytes and grab number of instances
	const size_t N = as_size_t[-1];

	// call instances' destructor in reverse order
	for (size_t i = N; i > 0; --i)
		as_T[i - 1].~T();

	allocator.Clear(as_size_t - 1);
}

//Implementation for POD-types.
template <typename T, class ALLOCATOR>
void DeleteArray(T* ptr, ALLOCATOR& allocator, PODType)
{
	allocator.Clear(ptr);
}
//-----------------------------------

template <typename T>
T nextMultiple(T multipleOf, T value)
{
	T multiple = value + multipleOf - 1;
	multiple -= (multiple % multipleOf);
	return multiple;
}