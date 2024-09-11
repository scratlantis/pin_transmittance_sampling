#include <vka/core/core_common.h>
#include <unordered_map>

namespace vka
{
	class DataCache
	{
		std::unordered_map<hash_t, Buffer> map;

		public:

		void clear();
		bool fetch(Buffer &buf, hash_t key);
	};
}