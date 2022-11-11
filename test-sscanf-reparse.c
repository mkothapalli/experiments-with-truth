#include <stdio.h>
#include <time.h>
#include <stdint.h>

#include <inttypes.h>
#include <netinet/in.h>

const char* shieldheader = "ttl=120; grace=0; hit=0; pass=0; esi=1; orig-ttl=120; cacheable=1; esi_allow_inside_cdata=0; req.grace=9223372037; stream_miss=0;msie=9223372037;mswr=9223372037;sie=0;swr=0;cpv=1;waited=0;pci=0;f=0;str=0;std=0;zh2o=1;hbto=470;bbto=0;vht=61200,0,16900,21500,0,0,0,0,0,0,0;vhc=1,0,2,1,0,0,0,0,0,0,0;bct=1668127593374309000,389100,163300,357500,0,0,0,0,414100,430800,559300,12896400,12961600;ben=;bip=0:72058143793676288;bpt=57768;preqs=0;bsz=34;obsz=34;tslu=0;";

#define VHT_STRINGIFY(num)      VHT_STRINGIFY_X(num)
#define VHT_STRINGIFY_X(num)    #num
#define BEN_STRINGIFY(num)      "."BEN_STRINGIFY_X(num)
#define BEN_STRINGIFY_X(num)    #num

int64_t
ns_to_s(int64_t ns)
{
        return ns / 1000000000;
}

int64_t
s_to_ns(int64_t s)
{
        if (s > ns_to_s(INT64_MAX)) {
                return INT64_MAX;
        } else if (s < ns_to_s(INT64_MIN)) {
                return INT64_MIN;
        }

        return s * 1000000000;
}

int64_t
sat_add(int64_t num1, int64_t num2)
{
        /* prevent underflow when adding two negative numbers */
        if ((num1 < 0 && num2 < 0) && (num1 - INT64_MIN + num2 <= 0)) {
                return INT64_MIN;
        }
        /* prevent overflow when adding two positive numbers */
        if ((num1 > 0 && num2 > 0) && (INT64_MAX - num1 - num2 <= 0)) {
                return INT64_MAX;
        }
        return num1 + num2;
}

int64_t
TIM_mono_ns(void)
{
        struct timespec ts;
        int r;
        r = clock_gettime(CLOCK_MONOTONIC, &ts);
        return sat_add(s_to_ns(ts.tv_sec), ts.tv_nsec);
}

#define MAX_BENAME_FROM_CLUSTERING_RESP 255

int main() {
	int64_t start=0, end=0;

        double ttl_s = 0., grace_s = 0., origttl_s = 0., req_grace_s = 0., max_stale_if_error_s = 0., max_stale_while_revalidate_s = 0.;
        double stale_if_error_s = 0., stale_while_revalidate_s = 0.;
        int64_t time_since_lastuse_s = 0;
	int n;
        int hit = 0, pass = 0, esi = 0, ret = 0, cacheable = 0, esiincdata = 0, stream_miss = 0, cpv = 0, waited = 0, nvht = 0, nvhc = 0;
        unsigned pci = 0, cres_flags = 0, zh2o = 0, preqs = 0;
        long unsigned hbto = 0, bbto = 0, bsz = 0, obsz = 0;
	struct sockaddr_in6 beaddr = { 0 };
        double smiss_throttle_delay_s = 0.;
        long unsigned smiss_throttle_readsz = 0;

#define MAX_VHT_CHARACTERS      800
#define MAX_VHC_CHARACTERS      500
#define MAX_BET_CHARACTERS      400     // 11 of int64_t
        char vhts[MAX_VHT_CHARACTERS + 1] = {0};
        char vhcs[MAX_VHC_CHARACTERS + 1] = {0};
        char bcts[MAX_BET_CHARACTERS + 1] = {0};
        char bename[MAX_BENAME_FROM_CLUSTERING_RESP + 1] = {0};

	start = TIM_mono_ns();
	for (int i = 0; i < 1000000; i++) {
		ret = sscanf(shieldheader, "ttl=%lf; grace=%lf; hit=%d; pass=%d; "
                    "esi=%d; orig-ttl=%lf; cacheable=%d; "
                    "esi_allow_inside_cdata=%d; req.grace=%lf; "
                    "stream_miss=%d;msie=%lf;mswr=%lf;"
                    "sie=%lf;swr=%lf;cpv=%d;waited=%d;pci=%u;f=%x;"
                    "str=%lu;std=%lf;zh2o=%u;hbto=%lu;bbto=%lu;"
                    "vht=%" VHT_STRINGIFY(MAX_VHT_CHARACTERS) "[^;];"
                    "vhc=%" VHT_STRINGIFY(MAX_VHC_CHARACTERS) "[^;];"
                    "bct=%" VHT_STRINGIFY(MAX_BET_CHARACTERS) "[^;];"
                    "ben=%" VHT_STRINGIFY(MAX_BENAME_FROM_CLUSTERING_RESP) "[^;];"
                    "bip=%"PRIu64":%"PRIu64";bpt=%hu;preqs=%u;bsz=%ld;obsz=%ld;tslu=%"PRIu64";",
                    &ttl_s, &grace_s, &hit, &pass, &esi, &origttl_s,
                    &cacheable, &esiincdata, &req_grace_s, &stream_miss,
                    &max_stale_if_error_s, &max_stale_while_revalidate_s,
                    &stale_if_error_s, &stale_while_revalidate_s, &cpv,
                    &waited, &pci, &cres_flags, &smiss_throttle_readsz,
                    &smiss_throttle_delay_s, &zh2o, &hbto, &bbto, vhts,
                    vhcs, bcts, bename,
                    (uint64_t*)&beaddr.sin6_addr.s6_addr[0],
                    (uint64_t*)&beaddr.sin6_addr.s6_addr[8],
                    &beaddr.sin6_port,
                    &preqs, &bsz, &obsz, &time_since_lastuse_s);

		if (ret == 26) {
                        ret = sscanf(shieldheader, "ttl=%lf; grace=%lf; hit=%d; pass=%d; "
                            "esi=%d; orig-ttl=%lf; cacheable=%d; "
                            "esi_allow_inside_cdata=%d; req.grace=%lf; "
                            "stream_miss=%d;msie=%lf;mswr=%lf;"
                            "sie=%lf;swr=%lf;cpv=%d;waited=%d;pci=%u;f=%x;"
                            "str=%lu;std=%lf;zh2o=%u;hbto=%lu;bbto=%lu;"
                            "vht=%" VHT_STRINGIFY(MAX_VHT_CHARACTERS) "[^;];"
                            "vhc=%" VHT_STRINGIFY(MAX_VHC_CHARACTERS) "[^;];"
                            "bct=%" VHT_STRINGIFY(MAX_BET_CHARACTERS) "[^;];"
                            "ben=;"
                            "bip=%"PRIu64":%"PRIu64";bpt=%hu;preqs=%u;bsz=%ld;obsz=%ld;tslu=%"PRIu64";",
                            &ttl_s, &grace_s, &hit, &pass, &esi, &origttl_s,
                            &cacheable, &esiincdata, &req_grace_s, &stream_miss,
                            &max_stale_if_error_s, &max_stale_while_revalidate_s,
                            &stale_if_error_s, &stale_while_revalidate_s, &cpv,
                            &waited, &pci, &cres_flags, &smiss_throttle_readsz,
                            &smiss_throttle_delay_s, &zh2o, &hbto, &bbto, vhts,
                            vhcs, bcts,
                            (uint64_t*)&beaddr.sin6_addr.s6_addr[0],
                            (uint64_t*)&beaddr.sin6_addr.s6_addr[8],
                            &beaddr.sin6_port,
                            &preqs, &bsz, &obsz, &time_since_lastuse_s);

                        /* We tell the rest of the system that we
                         * actually expected ben= to be empty by
                         * incrementing the ret by one. This causes
                         * the rest of this function to behave as if
                         * the ben= was filled in, but was empty. */
                        ret += 1;
                }
	}

	end = TIM_mono_ns();
	printf("Approach 1 took %ld seconds\n", ns_to_s(end - start));

	return 0;
}
