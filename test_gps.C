#include "test_gps.h"
#include <iostream>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <thread>

enum TimeFormat { LOCALTIME, UTC, UNIX, ISO_8601 };

auto TimespecToTimeStr(const timespec& gpsd_time, TimeFormat time_format = LOCALTIME) {
  // example showing different ways of formating GPSD's timespec, depending on requirement
  std::ostringstream oss;
  switch (time_format) {
    case LOCALTIME: {
      // convert timespec_t into localtime (dd-mm-YY HH:MM:SS)
      const auto tm = *std::localtime(&gpsd_time.tv_sec);
      oss << std::put_time(&tm, "%d-%m-%Y %H:%M:%S");
      break;
    }
    case UTC: {
      // convert timespec_t into UTC (dd-mm-YY HH:MM:SS)
      const auto tm = *std::gmtime(&gpsd_time.tv_sec);
      oss << std::put_time(&tm, "%d-%m-%Y %H:%M:%S");
      break;
    }
    case UNIX:
      // returns seconds since the Epoch
      oss << gpsd_time.tv_sec;
      break;
    case ISO_8601: {
      // convert timespec_t into ISO8601 UTC time (yyyy-MM-dd'T'HH:mm:ss'Z')
      constexpr size_t kScrSize{128};
      std::array<char, kScrSize> scr{};
      timespec_to_iso8601(gpsd_time, scr.data(), kScrSize);
      oss << scr.data();
      break;
    }
  }
  return oss.str();
}


bool astrogruff::gps::GPSD::connect() {
    if (gps == nullptr)
    {
        gps = std::make_unique<gpsmm>("localhost", DEFAULT_GPSD_PORT);
    }
    if (gps->stream(WATCH_ENABLE | WATCH_JSON) == nullptr)
    {
	std::cerr << "No GPSD running." << std::endl;
        return false;
    }
    return true;
}

void astrogruff::gps::GPSD::poll() {
	struct gps_data_t *gpsd_data;

	// initial wait
	if (!gps->waiting(1000000)) {
		std::cerr << "After a second there is is no data!" << std::endl;
		return;
	}

	constexpr auto kWaitingTime{1000000};

	for (;;) {
		if (!gps->waiting(kWaitingTime)) {
			continue;
		}

		if (gps->read() == nullptr) {
			std::cerr << "GPSD read error" << std::endl;
			return;
		}

		while (((gpsd_data = gps->read()) == nullptr) || (gpsd_data->fix.mode < MODE_2D)) {
			std::this_thread::sleep_for(std::chrono::seconds(1));
		}

    const auto status{gpsd_data->fix.status};
    const auto mode{gpsd_data->fix.mode};
    const auto latitude{gpsd_data->fix.latitude};
    const auto longitude{gpsd_data->fix.longitude};
    const auto hdop{gpsd_data->dop.hdop};
    const auto vdop{gpsd_data->dop.vdop};
    const auto pdop{gpsd_data->dop.pdop};
    const auto s_vis{gpsd_data->satellites_visible};
    const auto s_used{gpsd_data->satellites_used};
    const auto time_str{TimespecToTimeStr(gpsd_data->fix.time, ISO_8601)};  // you can change the 2nd argument to LOCALTIME, UTC, UNIX or ISO8601

    std::cout << std::setprecision(8) << std::fixed;  // set output to fixed floating point, 8 decimal precision
    std::cout << status << "," << mode << "," << time_str << "," << latitude << "," << longitude << "," << hdop << "," << vdop << "," << pdop << "," << s_vis << "," << s_used << '\n';
	}


}

int main() {
	astrogruff::gps::GPSD gpsd;

	if (gpsd.connect()) {
		gpsd.poll();
	}
}

