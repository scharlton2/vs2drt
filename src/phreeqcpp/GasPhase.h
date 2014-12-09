#if !defined(GASPHASE_H_INCLUDED)
#define GASPHASE_H_INCLUDED

#include <cassert>				// assert
#include <map>					// std::map
#include <string>				// std::string
#include <list>					// std::list
#include <vector>				// std::vector

#include "NumKeyword.h"
#include "NameDouble.h"
class cxxMix;

class cxxGasPhase:public cxxNumKeyword
{

  public:
	cxxGasPhase();
	cxxGasPhase(struct gas_phase *);
	  cxxGasPhase(PHREEQC_PTR_ARG_COMMA const std::map < int, cxxGasPhase > &entity_map,
				  cxxMix & mx, int n_user);
	 ~cxxGasPhase();

	enum GP_TYPE
	{
		GP_PRESSURE = 0,
		GP_VOLUME = 1
	};

	struct gas_phase *cxxGasPhase2gas_phase(PHREEQC_PTR_ARG);

	struct gas_comp *cxxGasPhaseComp2gas_comp(PHREEQC_PTR_ARG);

	//void dump_xml(std::ostream& os, unsigned int indent = 0)const;

	void dump_raw(std::ostream & s_oss, unsigned int indent) const;

	void read_raw(PHREEQC_PTR_ARG_COMMA CParser & parser, bool check = true);

#ifdef USE_MPI
	void mpi_pack(std::vector < int >&ints, std::vector < double >&doubles);
	void mpi_unpack(int *ints, int *ii, double *doubles, int *dd);
#endif

	void totalize(PHREEQC_PTR_ARG);

	const cxxNameDouble & get_totals() const
	{
		return this->totals;
	};

private:
	void add(const cxxGasPhase & addee, double extensive);

protected:
	cxxNameDouble gasPhaseComps;
	GP_TYPE type;
	double total_p;
	double volume;
	cxxNameDouble totals;

public:
	//static std::map<int, cxxGasPhase>& map;

};

#endif // !defined(GASPHASE_H_INCLUDED)
