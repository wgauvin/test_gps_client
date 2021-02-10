#ifndef __TEST_GPS_H__

#include <memory>
#include <libgpsmm.h>

namespace astrogruff {
	namespace gps {

class GPSD {

	private:
		std::unique_ptr<gpsmm> gps;

	public:
		bool connect();
		void poll();
};

	}
}

#endif

