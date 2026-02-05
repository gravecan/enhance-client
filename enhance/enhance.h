#include <sdk/includes.h>
#include "globals/globals.h"

namespace enhance
{
	class enhance_client
	{
	private:
		JNIEnv* env;
		JavaVM* vm;

	public:
		bool attach();
		void run();
		void unload();

		const auto get_env() { return env; }
		const auto get_java_vm() { return vm; }
	};

	extern std::unique_ptr<enhance_client> instance;
}

