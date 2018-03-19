/*****************************************************************************************
 *                                                                                       *
 * OpenSpace                                                                             *
 *                                                                                       *
 * Copyright (c) 2014-2018                                                               *
 *                                                                                       *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this  *
 * software and associated documentation files (the "Software"), to deal in the Software *
 * without restriction, including without limitation the rights to use, copy, modify,    *
 * merge, publish, distribute, sublicense, and/or sell copies of the Software, and to    *
 * permit persons to whom the Software is furnished to do so, subject to the following   *
 * conditions:                                                                           *
 *                                                                                       *
 * The above copyright notice and this permission notice shall be included in all copies *
 * or substantial portions of the Software.                                              *
 *                                                                                       *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,   *
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A         *
 * PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT    *
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF  *
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE  *
 * OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                                         *
 ****************************************************************************************/

#include <modules/exoplanets/exoplanetsmodule.h>

#include <openspace/documentation/documentation.h>
#include <openspace/engine/openspaceengine.h>

//#include <openspace/rendering/renderable.h>
//#include <openspace/util/factorymanager.h>

#include <ghoul/misc/assert.h>

#include <fstream>

namespace openspace {

ExoplanetsModule::ExoplanetsModule() : OpenSpaceModule(Name) {}

std::vector<float> readSpeckFile(std::string starname) {
    std::ifstream file("C:/Users/Karin/Documents/OpenSpace/modules/exoplanets/stars.speck");
	  if (!file.good()) {
		    std::cout << "Failed to open Speck file";
	  }

	   int _nValuesPerStar = 0;

  	// The beginning of the speck file has a header that either contains comments
  	// (signaled by a preceding '#') or information about the structure of the file
  	// (signaled by the keywords 'datavar', 'texturevar', and 'texture')
	   std::string line = "";
	   while (true) {
		 std::streampos position = file.tellg();
		 std::getline(file, line);

     if (line[0] == '#' || line.empty()) {
       continue;
		 }

		if (line.substr(0, 7) != "datavar" &&
			line.substr(0, 10) != "texturevar" &&
			line.substr(0, 7) != "texture")
		{
			// we read a line that doesn't belong to the header, so we have to jump back
			// before the beginning of the current line
			file.seekg(position);
			break;
		}

		if (line.substr(0, 7) == "datavar") {
			// datavar lines are structured as follows:
			// datavar # description
			// where # is the index of the data variable; so if we repeatedly overwrite
			// the 'nValues' variable with the latest index, we will end up with the total
			// number of values (+3 since X Y Z are not counted in the Speck file index)
			std::stringstream str(line);

			std::string dummy;
			str >> dummy;
			str >> _nValuesPerStar;
			_nValuesPerStar += 1; // We want the number, but the index is 0 based
		}
	}

	_nValuesPerStar += 3; // X Y Z are not counted in the Speck file indices
    std::vector<float> coords;

	do {
        std::vector<float> temp(_nValuesPerStar);

		std::getline(file, line);
		std::stringstream str(line);

		for (int i = 0; i < _nValuesPerStar; ++i) {
			str >> temp[i];
		}

		std::string next;
		std::getline(str, next);
		std::stringstream namestr(next);

		std::string value;
        std::vector<std::string> names;

		while (namestr >> value) {
			if (value == "#" || value == "|")
				continue;
			else {
				names.push_back(value);
			}
		}

		for (size_t i = 0; i < names.size(); ++i) {
			if (names[i].compare(starname) == 0) {
                for (int i = 0; i < 3; i++) {
                    coords.push_back(temp[i]);
                }
                return coords;
			}
		}

	} while (!file.eof());

    return coords;
}

int addNode(lua_State* L) {

    const int StringLocation = -1;
    const std::string starname = luaL_checkstring(L, StringLocation);

    std::vector<float> values = readSpeckFile(starname);

    if (!values.empty())
    {
        double parsecinmeter = 3.08567758*10e16;

        const std::string luaTableParent = "{"
            "Name = '" + starname + "',"
            "Parent = 'SolarSystemBarycenter',"
            "Transform = {"
                "Translation = {"
                    "Type = 'StaticTranslation',"
                    "Position = {" + std::to_string(values[0] * parsecinmeter) + ", " + std::to_string(values[1] * parsecinmeter) + ", " + std::to_string(values[2] * parsecinmeter) + "}"
                "}"
            "}"
        "}";
        const std::string luaTableStarGlare = "{"
            "Name = '" + starname + "Plane',"
            "Parent = '" + starname + "',"
            "Renderable = {"
                "Type = 'RenderablePlaneImageLocal',"
                "Size = 1.3*10^10.5,"
                "Billboard = true,"
                "Texture = 'C:/Users/Karin/Documents/OpenSpace/modules/exoplanets/glare.png',"
                "BlendMode = 'Additive'"
            "}"
        "}";
        const std::string scriptParent = "openspace.addSceneGraphNode(" + luaTableParent + "); openspace.addSceneGraphNode(" + luaTableStarGlare + ");";
        OsEng.scriptEngine().queueScript(
            scriptParent,
            openspace::scripting::ScriptEngine::RemoteScripting::Yes
        );
    }
    else
    {
        printf("No star with that name.");
    }

}

scripting::LuaLibrary ExoplanetsModule::luaLibrary() const {

    scripting::LuaLibrary res;
    res.name = "exoplanets";
    res.functions = {
        {
            "addNode",
            &addNode,
            {},
            "string",
            "Adds two nodes to the scenegraph, one position node and one node to represenet the star."
        }

    };

    return res;
}

//void ExoplanetsModule::internalInitialize(const ghoul::Dictionary&) {
    //auto fRenderable = FactoryManager::ref().factory<Renderable>();
    //ghoul_assert(fRenderable, "No renderable factory existed");

    //fRenderable->registerClass<RenderableDebugPlane>("RenderableDebugPlane");
//}

//std::vector<documentation::Documentation> ExoplanetsModule::documentations() const {
    //return {
        //RenderableDebugPlane::Documentation()
    //};
//}


} // namespace openspace
