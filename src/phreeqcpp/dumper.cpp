#include <algorithm>			// std::replace

#include "dumper.h"
#include "Parser.h"

dumper::dumper(void)
{
	this->file_name = "dump.out";
	this->append = false;
	this->on = false;
}
dumper::dumper(CParser & parser)
{
	this->file_name = "dump.out";
	this->append = false;
	this->Read(parser);
}

dumper::~dumper(void)
{
}

void dumper::SetAll(bool tf)
{
	this->binList.SetAll(tf);

}
#ifdef SKIP
bool dumper::Read(CParser & parser)
{

	bool return_value(true);
	static std::vector < std::string > vopts;
	if (vopts.empty())
	{
		vopts.reserve(15);
		vopts.push_back("solution");
		vopts.push_back("pp_assemblage");
		vopts.push_back("equilibrium_phases");
		vopts.push_back("exchange");
		vopts.push_back("surface");
		vopts.push_back("s_s_assemblage");
		vopts.push_back("gas_phase");
		vopts.push_back("kinetics");
		vopts.push_back("file");
		vopts.push_back("append");
		vopts.push_back("all");		
	}

	std::istream::pos_type ptr;
	std::istream::pos_type next_char;
	std::string token;
	int opt_save;
	bool useLastLine(false);

	// Read mix number and description
	//this->read_number_description(parser);

	opt_save = CParser::OPT_DEFAULT;

	for (;;)
	{
		int opt;
		if (useLastLine == false)
		{
			opt = parser.get_option(vopts, next_char);
		}
		else
		{
			opt = parser.getOptionFromLastLine(vopts, next_char);
		}
		if (opt == CParser::OPT_DEFAULT)
		{
			opt = opt_save;
		}
		else
		{
			opt_save = opt;
		}
		std::set < int > accumulator;

		// Read list of numbers or number ranges for line
		if (opt >= 0 && opt <= 7)
		{
			for (;;)
			{ 
				CParser::TOKEN_TYPE j = parser.copy_token(token, next_char);
				std::set < int > temp_set;
				int i; 
				if (j == CParser::TT_EMPTY)
				{
					break;
				}
				else if (j == CParser::TT_DIGIT)
				{
					std::replace(token.begin(), token.end(), '-', ' ');
					//if (token.find_last_of("-") != token.end())
					//{
					//	token.replace(token.find_last_of("-"), token.find_last_of("-") + 1, " ");
					//}

					std::istringstream iss(token);
					if (iss >> i)
					{
						temp_set.insert(i);
						if (iss >> i)
						{
							temp_set.insert(i);
						}
					}
					if (temp_set.size() == 1)
					{
						accumulator.insert(*(temp_set.begin()));
					}
					else if (temp_set.size() == 2)
					{
						int i1, i2;
						std::set <int>::iterator it;
						it = temp_set.begin();
						i1 = *it;
						it++;
						i2 = *it;
						for (i = i1; i <= i2; i++)
						{
							accumulator.insert(i);
						}
					}
					else
					{
						parser.error_msg("Expected positive integers for dump range.",
							CParser::OT_CONTINUE);
					}
				}
				else
				{
					parser.error_msg("Expected single number or range of numbers.",
						CParser::OT_CONTINUE);
				}
			}
		}

		// Process identifiers
		std::set < int >::iterator it;
		switch (opt)
		{
		case CParser::OPT_EOF:
			break;
		case CParser::OPT_KEYWORD:
			break;
		case CParser::OPT_DEFAULT:
		case CParser::OPT_ERROR:
			opt = CParser::OPT_EOF;
			parser.error_msg("Unknown input reading DUMP definition.",
							 CParser::OT_CONTINUE);
			parser.error_msg(parser.line().c_str(), CParser::OT_CONTINUE);
			useLastLine = false;
			return_value = false;
			break;
		case 0:
			for (it = accumulator.begin(); it != accumulator.end(); it++)
			{
				this->solution.insert(*it);
			}
			this->bool_solution = true;
			break;
		case 1:
		case 2:
			for (it = accumulator.begin(); it != accumulator.end(); it++)
			{
				this->pp_assemblage.insert(*it);
			}
			this->bool_pp_assemblage = true;
			break;
		case 3:
			for (it = accumulator.begin(); it != accumulator.end(); it++)
			{
				this->exchange.insert(*it);
			}
			this->bool_exchange = true;
			break;
		case 4:
			for (it = accumulator.begin(); it != accumulator.end(); it++)
			{
				this->surface.insert(*it);
			}
			this->bool_surface = true;
			break;
		case 5:
			for (it = accumulator.begin(); it != accumulator.end(); it++)
			{
				this->s_s_assemblage.insert(*it);
			}			
			this->bool_s_s_assemblage = true;
			break;
		case 6:
			for (it = accumulator.begin(); it != accumulator.end(); it++)
			{
				this->gas_phase.insert(*it);
			}			
			this->bool_gas_phase = true;
			break;
		case 7:
			for (it = accumulator.begin(); it != accumulator.end(); it++)
			{
				this->kinetics.insert(*it);
			}
			this->bool_kinetics = true;
			break;
		case 8:				//file
			std::getline(parser.get_iss(), this->file_name);
			this->file_name = trim(this->file_name, " \t");
			if (this->file_name.size() == 0)
			{
				this->file_name = "dump.out";
			}

			break;
		case 9:				//append
			{
				CParser::TOKEN_TYPE j = parser.copy_token(token, next_char);
				//if (!(parser.get_iss() >> this->append))
				this->append = true;
				if (token.c_str()[0] == 'f' || token.c_str()[0] == 'F')
				{
					this->append = false;
				}
			}
			break;
		case 10:			//all
			this->DumpAll(true);
			break;
		}

		
		if (opt == CParser::OPT_EOF || opt == CParser::OPT_KEYWORD)
			break;
	}
	return(return_value);
}
#endif
bool dumper::Read(CParser & parser)
{

	bool return_value(true);
	static std::vector < std::string > vopts;
	if (vopts.empty())
	{
		vopts.reserve(20);
		vopts.push_back("solution");
		vopts.push_back("solutions");
		vopts.push_back("pp_assemblage");
		vopts.push_back("pp_assemblages");
		vopts.push_back("equilibrium_phase");
		vopts.push_back("equilibrium_phases");
		vopts.push_back("exchange");
		vopts.push_back("surface");
		vopts.push_back("s_s_assemblage");
		vopts.push_back("solid_solution");
		vopts.push_back("solid_solutions");
		vopts.push_back("gas_phase");
		vopts.push_back("gas_phases");
		vopts.push_back("kinetics");
		vopts.push_back("file");
		vopts.push_back("append");
		vopts.push_back("all");	
		vopts.push_back("cell");
		vopts.push_back("cells");			// 18
		vopts.push_back("mix");				// 19
		vopts.push_back("reaction");		// 20
		vopts.push_back("reactions");		// 21
		vopts.push_back("temperature");		// 22
		vopts.push_back("reaction_temperature");	// 23
		vopts.push_back("reaction_temperatures");   // 24
	}

	std::istream::pos_type ptr;
	std::istream::pos_type next_char;
	std::string token;
	int opt_save;
	bool useLastLine(false);

	// Read mix number and description
	//this->read_number_description(parser);

	opt_save = CParser::OPT_DEFAULT;

	for (;;)
	{
		int opt;
		StorageBinListItem cells;
		opt = parser.get_option(vopts, next_char);
		if (opt == CParser::OPT_DEFAULT)
		{
			opt = opt_save;
		}
		else
		{
			opt_save = opt;
		}

		// Select StorageBinListItem
		StorageBinListItem *item = NULL;
		switch (opt)
		{
		case 0:
		case 1:
			item = &(this->binList.Get_solution());
			break;
		case 2:
		case 3:
		case 4:
		case 5:
			item = &(this->binList.Get_pp_assemblage());
			break;
		case 6:
			item = &(this->binList.Get_exchange());
			break;
		case 7:
			item = &(this->binList.Get_surface());
			break;
		case 8:
		case 9:
		case 10:
			item = &(this->binList.Get_s_s_assemblage());
			break;
		case 11:
		case 12:
			item = &(this->binList.Get_gas_phase());
			break;
		case 13:
			item = &(this->binList.Get_kinetics());
			break;
		case 17:	// cell
		case 18:	// cells
			item = &cells;
			break;
		case 19:	// mix
			item = &(this->binList.Get_mix());
			break;
		case 20:	// reaction
		case 21:	// reactions
			item = &(this->binList.Get_reaction());
			break;
		case 22:	// temperature
		case 23:	// reaction_temperature
		case 24:    // reaction_temperatures
			item = &(this->binList.Get_temperature());
			break;
		default:
			break;
		}

		// Read dump entity list of numbers or number ranges for line, store in item
		if ((opt >= 0 && opt <= 13) || (opt >= 17 && opt <= 23))
		{
			for (;;)
			{ 
				CParser::TOKEN_TYPE j = parser.copy_token(token, next_char);
				if (j == CParser::TT_DIGIT)
				{
					item->Augment(token);
				}
				else if (j == CParser::TT_EMPTY)
				{
					item->Augment(token);
					break;
				}
				else
				{
					parser.error_msg("Expected single number or range of numbers.",
						CParser::OT_CONTINUE);
				}
			}
		}

		if (opt == 17 || opt == 18)
		{
			this->binList.TransferAll(cells);
		}
		// Process other identifiers
		std::set < int >::iterator it;
		switch (opt)
		{
		case CParser::OPT_EOF:
			break;
		case CParser::OPT_KEYWORD:
			break;

		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
		case 8:
		case 9:
		case 10:
		case 11:
		case 12:
		case 13:
		case 17:
		case 18:
		case 19:
		case 20:
		case 21:
		case 22:
		case 23:
			break;
		case 14:				//file
			std::getline(parser.get_iss(), this->file_name);
			this->file_name = trim(this->file_name, " \t");
			if (this->file_name.size() == 0)
			{
				this->file_name = "dump.out";
			}

			break;
		case 15:				//append
			{
				parser.copy_token(token, next_char);
				//if (!(parser.get_iss() >> this->append))
				this->append = true;
				if (token.c_str()[0] == 'f' || token.c_str()[0] == 'F')
				{
					this->append = false;
				}
			}
			break;
		case 16:			//all
			this->SetAll(true);
			break;
		default:
		case CParser::OPT_DEFAULT:
		case CParser::OPT_ERROR:
			opt = CParser::OPT_EOF;
			parser.error_msg("Unknown input reading DUMP definition.",
							 CParser::OT_CONTINUE);
			parser.error_msg(parser.line().c_str(), CParser::OT_CONTINUE);
			useLastLine = false;
			return_value = false;
			break;
		}

		
		if (opt == CParser::OPT_EOF || opt == CParser::OPT_KEYWORD)
			break;
	}
	return(return_value);
}

bool dumper::Get_bool_any(void)
{
	return (
		Get_bool_solution()			||
		Get_bool_pp_assemblage()	||
		Get_bool_exchange()			||
		Get_bool_surface()			||
		Get_bool_s_s_assemblage()	||
		Get_bool_gas_phase()		||
		Get_bool_kinetics()			||
		Get_bool_mix()				||
		Get_bool_reaction()			||
		Get_bool_temperature()
		);
}
