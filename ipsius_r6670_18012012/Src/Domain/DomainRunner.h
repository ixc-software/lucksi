#ifndef __DOMAINRUNNER__
#define __DOMAINRUNNER__

namespace Domain
{

    class DomainStartup;

    // ����� ��� �������� � ��������� ������
    class DomainRunner
    {
    public:

        DomainRunner()
        {
        }

        bool Run(DomainStartup &params);
    };
	
	
}  // namespace Domain


#endif

