#define VMA_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include <vka/vka.h>
//#include <vka/resource_objects/DescriptorSetLayout.h>
#include <stdio.h>
#include <iostream>
#include <unordered_map>

// make const
//struct AppConstData
//{
//	// buffers
//	// images
//	// samplers
//	// framebuffers
//	// entities (desc data references, pipeline config, renderpass config, model data ref, instance data ref, instance count reference)
//};

//struct AppVarData
//{
//	// camera
//	// gState
//	// gVars
//	// instance data
//	// instance counts
//};


//struct Config
//{
//	// pin parameters
//	// asset paths
//	// options
//};

// cached
//struct RenderData
//{
//	// render passes, pipelines, shaders, descriptor set layouts, pipeline layouts
//};


// 1. Init app data
// 2. Main loop Start
// 3. Read config
// 4. Load/Update app data
// 5. Render
// 6. Main loop End
// 7. Cleanup


// static methods that fill resources based on config


// todo: auto resources, cachable render passes

//#define DEFINE_EQUALS_OVERLOAD(CHILD_TYPE, PARENT_TYPE)             \
//	bool CHILD_TYPE## ::operator==(const PARENT_TYPE & other) const \
//	{                                                               \
//		if (typeid(*this) == typeid(other))                         \
//		{                                                           \
//			return *this == static_cast<const CHILD_TYPE &>(other); \
//		}                                                           \
//		return false;                                               \
//	}

#define DEFINE_NOT_EQUALS_OVERLOAD(CHILD_TYPE, PARENT_TYPE)            \
	bool CHILD_TYPE## ::operator!=(const PARENT_TYPE & other) const    \
	{                                                                  \
		if (typeid(*this) == typeid(other))                            \
		{                                                              \
			return !(*this == static_cast<const CHILD_TYPE &>(other)); \
		}                                                              \
		return true;                                                   \
	}


//{
//	if (typeid(*this) OPERATOR typeid(other))
//	{
//		return *this OPERATOR static_cast<const CHILD_TYPE &>(other);
//	}
//	return false;
//}


struct Parent
{
	virtual bool operator==(const Parent& other) const = 0;
	virtual bool operator!=(const Parent &other) const = 0;
	virtual uint64_t hash() const = 0;
};

namespace std
{
template <>
struct hash<Parent>
{
	size_t operator()(Parent const &r) const
	{
		return static_cast<size_t>(r.hash());
	}
};
} // namespace std



struct Son : public Parent
{
	int  attribute;
	bool operator==(const Parent &other) const;
	bool operator!=(const Parent &other) const;
	uint64_t hash() const;
	bool operator==(const Son &other) const;
};



struct Daughter : public Parent
{
	int  attribute;
	bool operator==(const Parent &other) const;
	bool operator!=(const Parent &other) const;
	uint64_t hash() const;
	bool operator==(const Daughter &other) const;

};

bool Son::operator == (const Son &other) const
{
	std::cout << "Son == Son" << std::endl;
	return this->attribute == other.attribute;
}
bool Daughter::operator == (const Daughter &other) const
{
	std::cout << "Daughter == Daughter" << std::endl;
	return this->attribute == other.attribute;
}
uint64_t Son::hash() const
{
	return 0;
}
uint64_t Daughter::hash() const
{
	return 0;
}

DEFINE_EQUALS_OVERLOAD(Son, Parent)
DEFINE_NOT_EQUALS_OVERLOAD(Son, Parent)
DEFINE_EQUALS_OVERLOAD(Daughter, Parent)
DEFINE_NOT_EQUALS_OVERLOAD(Daughter, Parent)

template <class T>
struct PointerObjHash
{
	bool operator()(T const *a) const
	{
		std::hash<T> h;
		return h(*a);
	}
};
template <class T>
struct PointerObjEq
{
	bool operator()(T const *a, T const *b) const
	{
		return *a == *b;
	}
};

vka::AppState gState;

int main()
{
	std::unordered_map<Parent*, int, PointerObjHash<Parent>, PointerObjEq<Parent>> map;

	std::cout << "Hello, World!" << std::endl;
	Parent *sonA      = new Son();
	Parent *sonB      = new Son();
	Parent *daughterA = new Daughter();
	Parent *daughterB = new Daughter();

	*sonA == *sonB;
	std::cout << "----" << std::endl;
	*sonA == *daughterA;
	std::cout << "----" << std::endl;
	*daughterA == *sonB;
	std::cout << "----" << std::endl;
	*daughterA == *daughterB;



}