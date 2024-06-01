//#define MAKE_VERTEX_1ARG(CLASS_NAME, ARG1_TYPE, ARG1_NAME, ARG1_LAYOUT)                                  \
//	class CLASS_NAME                                                                                     \
//	{                                                                                                    \
//	  public:                                                                                            \
//		ARG1_TYPE ARG1_NAME;                                                                             \
//		CLASS_NAME(                                                                                      \
//		    ARG1_TYPE ARG1_NAME) :                                                                       \
//		    ARG1_NAME(ARG1_NAME){};                                                                      \
//		CLASS_NAME(){};                                                                                  \
//		bool operator==(const CLASS_NAME &other) const                                                   \
//		{                                                                                                \
//			return ARG1_NAME == other.ARG1_NAME;                                                         \
//		}                                                                                                \
//		static VkVertexInputBindingDescription getVertexBindingDescription(uint32_t binding)             \
//		{                                                                                                \
//			return vka::getVertexBindingDescription(binding, sizeof(CLASS_NAME));                        \
//		}                                                                                                \
//		static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions(uint32_t binding) \
//		{                                                                                                \
//			return vka::getAttributeDescriptions(binding, ARG1_LAYOUT, offsetof(CLASS_NAME, ARG1_NAME)); \
//		}                                                                                                \
//	};                                                                                                   \
//	namespace std                                                                                        \
//	{                                                                                                    \
//	template <>                                                                                          \
//	struct hash<CLASS_NAME>                                                                              \
//	{                                                                                                    \
//		size_t operator()(CLASS_NAME const &vert) const                                                  \
//		{                                                                                                \
//			size_t h1 = hash<glm::vec3>()(vert.ARG1_NAME);                                               \
//			return h1;                                                                                   \
//		}                                                                                                \
//	};                                                                                                   \
//	}
//
//#define MAKE_VERTEX_2ARG(CLASS_NAME,                                                                     \
//                         ARG1_TYPE, ARG1_NAME, ARG1_LAYOUT,                                              \
//                         ARG2_TYPE, ARG2_NAME, ARG2_LAYOUT)                                              \
//	class CLASS_NAME                                                                                     \
//	{                                                                                                    \
//	  public:                                                                                            \
//		ARG1_TYPE ARG1_NAME;                                                                             \
//		ARG2_TYPE ARG2_NAME;                                                                             \
//		CLASS_NAME(                                                                                      \
//		    ARG1_TYPE ARG1_NAME,                                                                         \
//		    ARG2_TYPE ARG2_NAME) :                                                                       \
//		    ARG1_NAME(ARG1_NAME),                                                                        \
//		    ARG2_NAME(ARG2_NAME){};                                                                      \
//		CLASS_NAME(){};                                                                                  \
//		bool operator==(const CLASS_NAME &other) const                                                   \
//		{                                                                                                \
//			return ARG1_NAME == other.ARG1_NAME && ARG2_NAME == other.ARG2_NAME;                         \
//		}                                                                                                \
//		static VkVertexInputBindingDescription getVertexBindingDescription(uint32_t binding)             \
//		{                                                                                                \
//			return vka::getVertexBindingDescription(binding, sizeof(CLASS_NAME));                        \
//		}                                                                                                \
//		static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions(uint32_t binding) \
//		{                                                                                                \
//			return vka::getAttributeDescriptions(binding                                                 \
//			                                         ARG1_LAYOUT,                                        \
//			                                     offsetof(CLASS_NAME, ARG1_NAME),                        \
//			                                     ARG2_LAYOUT, offsetof(CLASS_NAME, ARG2_NAME));          \
//		}                                                                                                \
//	};                                                                                                   \
//	namespace std                                                                                        \
//	{                                                                                                    \
//	template <>                                                                                          \
//	struct hash<CLASS_NAME>                                                                              \
//	{                                                                                                    \
//		size_t operator()(CLASS_NAME const &vert) const                                                  \
//		{                                                                                                \
//			size_t h = 0;                                                                                \
//			hashCombine(h, vert.ARG1_NAME);                                                              \
//			hashCombine(h, vert.ARG2_NAME);                                                              \
//			return h;                                                                                    \
//		}                                                                                                \
//	};                                                                                                   \
//	}
//
//#define MAKE_INSTANCE_1ARG(CLASS_NAME, ARG1_TYPE, ARG1_NAME, ARG1_LAYOUT)                                \
//	class CLASS_NAME                                                                                     \
//	{                                                                                                    \
//	  public:                                                                                            \
//		ARG1_TYPE ARG1_NAME;                                                                             \
//		CLASS_NAME(                                                                                      \
//		    ARG1_TYPE ARG1_NAME) :                                                                       \
//		    ARG1_NAME(ARG1_NAME){};                                                                      \
//		CLASS_NAME(){};                                                                                  \
//		static VkVertexInputBindingDescription getBindingDescription(uint32_t binding)                   \
//		{                                                                                                \
//			return vka::getInstanceBindingDescription(binding, sizeof(CLASS_NAME));                      \
//		}                                                                                                \
//		static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions(uint32_t binding) \
//		{                                                                                                \
//			return vka::getAttributeDescriptions(binding, ARG1_LAYOUT, offsetof(CLASS_NAME, ARG1_NAME)); \
//		}                                                                                                \
//	};                                                                                                   \
//	}
//
//#define MAKE_INSTANCE_2ARG(CLASS_NAME,                                                                   \
//                           ARG1_TYPE, ARG1_NAME, ARG1_LAYOUT,                                            \
//                           ARG2_TYPE, ARG2_NAME, ARG2_LAYOUT)                                            \
//	class CLASS_NAME                                                                                     \
//	{                                                                                                    \
//	  public:                                                                                            \
//		ARG1_TYPE ARG1_NAME;                                                                             \
//		ARG2_TYPE ARG2_NAME;                                                                             \
//		CLASS_NAME(                                                                                      \
//		    ARG1_TYPE ARG1_NAME,                                                                         \
//		    ARG2_TYPE ARG2_NAME) :                                                                       \
//		    ARG1_NAME(ARG1_NAME),                                                                        \
//		    ARG2_NAME(ARG2_NAME){};                                                                      \
//		CLASS_NAME(){};                                                                                  \
//		static VkVertexInputBindingDescription getBindingDescription(uint32_t binding)                   \
//		{                                                                                                \
//			return vka::getInstanceBindingDescription(binding, sizeof(CLASS_NAME));                      \
//		}                                                                                                \
//		static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions(uint32_t binding) \
//		{                                                                                                \
//			return vka::getAttributeDescriptions(binding,                                                \
//			                                     ARG1_LAYOUT, offsetof(CLASS_NAME, ARG1_NAME),           \
//			                                     ARG2_LAYOUT, offsetof(CLASS_NAME, ARG2_NAME));          \
//		}                                                                                                \
//	}
