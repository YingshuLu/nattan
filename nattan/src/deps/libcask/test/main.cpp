
#include <iostream>
#include "TmCrsMessage.h"


int main() {
	CrsSingleRequest req1;
	req1.setQueryServiceId();
	req1.setQueryServiceName();
	req1.setQueryValue("http://www.baidu.com");

	CrsSingleRequest req2;
	req2.setQueryServiceId();
	req2.setQueryServiceName();
	req2.setQueryType(CRS_INFORMATION::CRS_QUERY_BY_FQDN);
	req2.setQueryValue("google.com");

	CrsSingleRequest req3;
	req3.setQueryServiceId();
	req3.setQueryServiceName();
	req3.setQueryValue("https://www.google.com");
	
	CrsRequest request;
	request.append(req1);
	request.append(req2);
	request.append(req3);

	std::cout << request.toString() << std::endl;
	return 0;
}
