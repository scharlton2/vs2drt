#if !defined(ISOLUTIONCOMP_H_INCLUDED)
#define ISOLUTIONCOMP_H_INCLUDED

#include <string>
#include <map>					// std::map
#include <vector>
#include <set>
#include "Phreeqc_class.h"
// forward declarations
class cxxISolution;				// reqd for read and dump_xml

class cxxISolutionComp
{
  public:
	cxxISolutionComp(void);
	  cxxISolutionComp(struct conc *conc_ptr);
	 ~cxxISolutionComp(void);

  public:

	//STATUS_TYPE read(CParser& parser, CSolution& sol);

	void dump_xml(std::ostream & os, unsigned int indent = 0) const;

	const std::string &get_description() const
	{
		return this->description;
	}
	void set_description(char *l_description)
	{
		if (l_description != NULL)
			this->description = std::string(l_description);
		else
			this->description.clear();
	}

	double get_moles() const
	{
		return this->moles;
	}
	void set_moles(double l_moles)
	{
		this->moles = l_moles;
	}

	double get_input_conc() const
	{
		return this->input_conc;
	}
	void set_input_conc(double l_input_conc)
	{
		this->input_conc = l_input_conc;
	}

	std::string get_units()const
	{
		return this->units;
	}
	void set_units(char *l_units)
	{
		if (l_units != NULL)
			this->units = std::string(l_units);
		else
			this->units.clear();
	}

	const std::string &get_equation_name() const
	{
		return this->equation_name;
	}
	void set_equation_name(char *l_equation_name)
	{
		if (l_equation_name != NULL)
			this->equation_name = std::string(l_equation_name);
		else
			this->equation_name.clear();

	}

	double get_phase_si() const
	{
		return this->phase_si;
	}
	void set_phase_si(int l_phase_si)
	{
		this->phase_si = l_phase_si;
	}

	int get_n_pe() const
	{
		return this->n_pe;
	}
	void set_n_pe(int l_n_pe)
	{
		this->n_pe = l_n_pe;
	}

	const std::string &get_as() const
	{
		return this->as;
	}
	void set_as(char *l_as)
	{
		if (l_as != NULL)
			this->as = std::string(l_as);
		else
			this->as.clear();
	}

	//double get_gfw()const {return this->gfw;}
	double get_gfw() const
	{
		return this->gfw;
	};
	void set_gfw(double l_gfw)
	{
		this->gfw = l_gfw;
	}
	void set_gfw(PHREEQC_PTR_ARG);

	bool operator<(const cxxISolutionComp & conc) const
	{
		return ::strcmp(this->description.c_str(), conc.description.c_str()) < 0;
	}

	static struct conc *cxxISolutionComp2conc(PHREEQC_PTR_ARG_COMMA const std::map < std::string,	cxxISolutionComp > &t);

  private:
	  std::string description;
	  double moles;
	  double input_conc;
	  std::string units;
	  std::string equation_name;
	  double phase_si;
	  int n_pe;
	  std::string as;
	  double gfw;
};

#endif // ISOLUTIONCOMP_H_INCLUDED
