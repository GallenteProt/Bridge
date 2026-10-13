class CfgPatches
{
	class Bridge
	{
		units[] = {};
		weapons[] = {};
		requiredVersion = 0.1;
		requiredAddons[] = {"DZ_Data", "DZ_Scripts"};
	};
};

class CfgMods
{
	class Bridge
	{
		dir = "Bridge";
		picture = "";
		action = "";
		hideName = 1;
		hidePicture = 1;
		name = "Bridge";
		credits = "DayZ End";
		author = "DayZ End";
		authorID = "0";
		version = "1.0";
		extra = 0;
		type = "mod";
		dependencies[] = {"Game", "World", "Mission"};
		class defs
		{
			class gameScriptModule
			{
				value = "";
				files[] = {"Bridge/scripts/3_Game"};
			};
			class worldScriptModule
			{
				value = "";
				files[] = {"Bridge/scripts/4_World"};
			};
			class missionScriptModule
			{
				value = "";
				files[] = {"Bridge/scripts/5_Mission"};
			};
		};
	};
};
