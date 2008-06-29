
/* Config.h
     Header for MR_Config class. */

#pragma once

#include <string>

#ifdef MR_UTIL
#define MR_DllDeclare   __declspec( dllexport )
#else
#define MR_DllDeclare   __declspec( dllimport )
#endif

// Forward declarations.
namespace yaml {
	class Emitter;
}

/**
 * Global configuration manager, shared by all aspects of the system.
 */
class MR_Config
{
	private:
		static MR_Config *instance;
		std::string path;

	protected:
		MR_DllDeclare MR_Config(const std::string &file="");
	public:
		MR_DllDeclare virtual ~MR_Config();
	
		MR_DllDeclare static void Init(const std::string &path="");
		MR_DllDeclare static void Shutdown();

		MR_DllDeclare static std::string GetDefaultPath();
		MR_DllDeclare std::string GetConfigFilename() const;

	public:
		MR_DllDeclare static MR_Config *GetInstance() { return instance; }

		MR_DllDeclare void ResetToDefaults();

		MR_DllDeclare void Load();
		MR_DllDeclare void Save();

	private:
		MR_DllDeclare void SaveVersion(yaml::Emitter*);

	public:
		struct cfg_misc_t {
			bool displayFirstScreen;
			bool introMovie;

			MR_DllDeclare void Save(yaml::Emitter*);
		} misc;
};
