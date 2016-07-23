#include "sys_local.h"

namespace TimeDate {
	// Converts a Rapture_TimeDate into Unix time_t structure.
	time_t ConvertToUnix(Rapture_TimeDate in) {
		tm outtm{ in.second, in.minute, in.hour, in.day, in.month, in.year, /* DST? */ };
		return mktime(&outtm);
	}

	// Converts a Unix tm structure into a Rapture_TimeDate
	Rapture_TimeDate ConvertFromUnix(tm* in) {
		Rapture_TimeDate out{ in->tm_year, in->tm_mon, in->tm_mday, in->tm_hour, in->tm_min, in->tm_sec };
		return out;
	}

	// Gets the current date and time as Rapture_TimeDate structure.
	Rapture_TimeDate GetCurrent() {
		Rapture_TimeDate out = { 0 };
		time_t t = time(0);
		tm* now = localtime(&t);

		return ConvertFromUnix(now);
	}

	// Determines how long it has been between two Rapture_TimeDate. Output is stored in result variable.
	void Subtract(Rapture_TimeDate a, Rapture_TimeDate b, Rapture_TimeDate* result) {
		if (result == nullptr) {
			return;
		}

		time_t atime = ConvertToUnix(a);
		time_t btime = ConvertToUnix(b);
		time_t diff = abs(atime - btime);
		tm* difftm = localtime(&diff);

		Rapture_TimeDate out = ConvertFromUnix(difftm);
		memcpy(result, &out, sizeof(out));
	}

	// Adds one Rapture_TimeDate to another. Output is stored in result variable.
	void Add(Rapture_TimeDate a, Rapture_TimeDate b, Rapture_TimeDate* result) {
		if (result == nullptr) {
			return;
		}

		time_t atime = ConvertToUnix(a);
		time_t btime = ConvertToUnix(b);
		time_t sum = atime + btime;
		tm* sumtm = localtime(&sum);

		Rapture_TimeDate out = ConvertFromUnix(sumtm);
		memcpy(result, &out, sizeof(out));
	}
}