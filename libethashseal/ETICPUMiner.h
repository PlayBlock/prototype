#pragma once

#include "libdevcore/Worker.h"
#include <libethereum/GenericMiner.h>
//#include "EthashAux.h"
#include "ETIProofOfWork.h"


namespace dev
{
	namespace eth
	{

		class ETICPUMiner : public GenericMiner<ETIProofOfWork>, Worker
		{
		public:
			ETICPUMiner(GenericMiner<ETIProofOfWork>::ConstructionInfo const& _ci);
			~ETICPUMiner();

			static unsigned instances() { return s_numInstances > 0 ? s_numInstances : std::thread::hardware_concurrency(); }
			static std::string platformInfo();
			static void listDevices() {}
			static bool configureGPU(unsigned, unsigned, unsigned, unsigned, unsigned, bool, unsigned, uint64_t) { return false; }
			static void setNumInstances(unsigned _instances) { s_numInstances = std::min<unsigned>(_instances, std::thread::hardware_concurrency()); }

		protected:
			void kickOff() override;
			void pause() override;

		private:
			void workLoop() override;
			static unsigned s_numInstances;
		};

	}
}
