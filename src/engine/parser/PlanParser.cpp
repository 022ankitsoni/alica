/*
 * PlanParser.cpp
 *
 *  Created on: Mar 27, 2014
 *      Author: Stephan Opfer
 */
#include "engine/parser/PlanParser.h"
#define PP_DEBUG
namespace alica
{

	PlanParser::PlanParser(shared_ptr<PlanRepository> rep)
	{
		this->rep = rep;
		this->mf = shared_ptr<ModelFactory>(new ModelFactory(this, rep));
		this->sc = supplementary::SystemConfig::getInstance();
		this->domainConfigFolder = this->sc->getConfigPath();

		this->planDir = (*this->sc)["Alica"]->get<string>("Alica.PlanDir", NULL);
		this->roleDir = (*this->sc)["Alica"]->get<string>("Alica.RoleDir", NULL);

		if (domainConfigFolder.find_last_of("/") != domainConfigFolder.length() - 1)
		{
			domainConfigFolder = domainConfigFolder + "/";
		}
		if (planDir.find_last_of("/") != planDir.length() - 1)
		{
			planDir = planDir + "/";
		}
		if (roleDir.find_last_of("/") != roleDir.length() - 1)
		{
			roleDir = roleDir + "/";
		}
		if (!(supplementary::FileSystem::isPathRooted(this->planDir)))
		{
			basePlanPath = domainConfigFolder + planDir;
		}
		else
		{
			basePlanPath = planDir;
		}
		if (!(supplementary::FileSystem::isPathRooted(this->roleDir)))
		{
			baseRolePath = domainConfigFolder + roleDir;
		}
		else
		{
			baseRolePath = roleDir;
		}
		cout << "PP: basePlanPath: " << basePlanPath << endl;
		cout << "PP: baseRolePath: " << baseRolePath << endl;
		if (!(supplementary::FileSystem::fileExists(basePlanPath)))
		{
			AlicaEngine::getInstance()->abort("PP: BasePlanPath does not exists " + basePlanPath);
		}
		if (!(supplementary::FileSystem::fileExists(baseRolePath)))
		{
			AlicaEngine::getInstance()->abort("PP: BaseRolePath does not exists " + baseRolePath);
		}
	}

	PlanParser::~PlanParser()
	{
		// TODO Auto-generated destructor stub
	}

	shared_ptr<Plan> PlanParser::ParsePlanTree(string masterplan)
	{
		string masterPlanPath;
		bool found = supplementary::FileSystem::findFile(this->basePlanPath, masterplan + ".pml", masterPlanPath);
		cout << "PP: masterPlanPath: " << masterPlanPath << endl;
		if (!found)
		{
			AlicaEngine::getInstance()->abort("PP: Cannot find MasterPlan '" + masterplan);
		}
		this->currentFile = masterPlanPath;
		this->currentDirectory = supplementary::FileSystem::getParent(masterPlanPath);

		cout << "PP: CurFile: " << this->currentFile << " CurDir: " << this->currentDirectory << endl;

		this->masterPlan = parsePlanFile(masterPlanPath);
		//ParseFileLoop();
		//this.mf.ComputeReachabilities();
		//return this.masterPlan;
	}

	std::shared_ptr<Plan> PlanParser::parsePlanFile(string& planFile)
	{
#ifdef PP_DEBUG
		cout << "PP: parsing Plan file: " << planFile << endl;
#endif

		tinyxml2::XMLDocument doc;
		doc.LoadFile(planFile.c_str());
		if (doc.ErrorID() != tinyxml2::XML_NO_ERROR)
		{
			cout << "PP: doc.ErrorCode: " << tinyxml2::XMLErrorStr[doc.ErrorID()] << endl;
			throw new exception();
		}

		// Structure of the XML file:
		// - Element "PLAY"      the root Element, which is the
		//                       FirstChildElement of the Document
		// - - Element "TITLE"   child of the root PLAY Element
		// - - - Text            child of the TITLE Element

		// Navigate to the title, using the convenience function,
		// with a dangerous lack of error checking.
		const char* title =
				doc.FirstChildElement("alica:Plan")->FirstChildElement("states")->FirstChildElement("plans")->GetText();
		cout << "Text (1): " << title << endl;

		// Text is just another Node to TinyXML-2. The more
		// general way to get to the XMLText:
		tinyxml2::XMLText* textNode =
				doc.FirstChildElement("alica:Plan")->FirstChildElement("states")->FirstChildElement("plans")->FirstChild()->ToText();
		title = textNode->Value();
		cout << "Text (2): " << title << endl;

		std::shared_ptr<Plan> p = this->mf->createPlan(&doc);
//		Plan* p = this->mf->createPlan(node);
		return p;
	}

	shared_ptr<RoleSet> PlanParser::ParseRoleSet(string roleSetName, string roleSetDir)
	{

		shared_ptr<RoleSet> r;
		return r;
	}

	shared_ptr<map<long, alica::AlicaElement> > PlanParser::GetParsedElements()
	{

		shared_ptr<map<long, alica::AlicaElement> > map;
		return map;
	}

	void PlanParser::IgnoreMasterPlanId(bool val)
	{
		this->mf->setIgnoreMasterPlanId(val);
	}

	string PlanParser::getCurrentFile()
	{
		return this->currentFile;
	}

	void PlanParser::setCurrentFile(string currentFile)
	{
		if (currentFile.compare(0, basePlanPath.size(), basePlanPath))
		{
			this->currentFile = currentFile.substr(basePlanPath.size());
		}
		else if (currentFile.compare(0, baseRolePath.size(), baseRolePath))
		{
			this->currentFile = currentFile.substr(baseRolePath.size());
		}
		else
		{
			this->currentFile = currentFile;
		}
	}

	/**
	 * parse id of the plan
	 * @param node the given xml node
	 * @return id: returns the id from the plan
	 */
	long PlanParser::parserId(tinyxml2::XMLElement* node)
	{
		long id = -1;
		string idString = node->Attribute("id");
		if (idString.compare("") != 0)
		{
			try
			{
				id = stol(idString);
			}
			catch (exception e)
			{
				AlicaEngine::getInstance()->abort("PP: Cannot convert ID to long: " + idString + " WHAT?? " + e.what());
			}
			return id;
		}
		else
		{
			idString = node->Attribute("href");
			if (idString.compare("") != 0)
			{
				id = fetchId(idString, id);
				return id;

			}
			else
			{
				const tinyxml2::XMLNode* currNode = node->FirstChild();
				while (currNode)
				{
					const tinyxml2::XMLText* textNode = currNode->ToText();
					if(textNode){
						id = fetchId(textNode->Value(), id);
						return id;
					}

					currNode = currNode->NextSibling();
				}
			}
		}

		cerr << "Cannot resolve remote reference!\nAttributes of node in question are:" << endl;
		const tinyxml2::XMLAttribute* curAttribute = node->FirstAttribute();
		while (curAttribute)
		{
			cout << curAttribute->Name() << " : " << curAttribute->Value() << endl;
			curAttribute = curAttribute->Next();
		}
		AlicaEngine::getInstance()->abort("PP: Couldn't resolve remote reference: " + string(node->Name()));
		return -1;
	}

	/**
	 * Helper
	 * @param idString is a String that have to be converted in a long
	 * @param id the really really really (stopfer) long id
	 */
	long PlanParser::fetchId(const string& idString, long id)
	{
		int hashPos = idString.find_first_of("#");
		string locator = idString.substr(0, hashPos);
		if (!locator.empty())
		{
			//TODO: Maybe doesnt work... somewhere / put pls
			string path = this->currentDirectory + locator;
			list<string>::iterator findIterParsed = find(filesParsed.begin(), filesParsed.end(), path);
			list<string>::iterator findIterToParse = find(filesToParse.begin(), filesToParse.end(), path);
			if (findIterParsed != filesParsed.end() && findIterToParse != filesToParse.end())
			{
				cout << "PP: Adding " + path + " to parse queue " << endl;
				filesToParse.push_back(path);
			}
		}
		string tokenId = idString.substr(hashPos, idString.length() - hashPos);
		try
		{
			id = stol(tokenId);
		}
		catch (exception e)
		{
			AlicaEngine::getInstance()->abort("PP: Cannot convert ID to long: " + tokenId + " WHAT?? " + e.what());
		}
		return id;
	}
}
/* namespace Alica */