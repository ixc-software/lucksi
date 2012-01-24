/*
*   Конкретные типы пакетов третьего уровня
*/
#ifndef L3PACKETTYPES_H
#define L3PACKETTYPES_H

#include "L3Packet.h"
#include "L3PacketFactory.h"
#include "L3MessageType.h"

#include "Utils/ErrorsSubsystem.h"


namespace ISDN
{
    class L3Call;

	class PacketAlerting : public PacketType<L3MessgeType::ALERTING>
    {
        typedef PacketType<L3MessgeType::ALERTING> Base;
    public:

        PacketAlerting(IsdnInfra& infra, IeFactory& ieFactory, QVector<byte> l2data );
            
        PacketAlerting(IIsdnL3Internal& IL3, const CallRef &callref);
        
    private:

        bool ValidateMandatoryIes(const std::multiset<IeId>& exist) const; // override
      
        void UpRout(L3Call &);  // override

        bool IeCorrectForThisMsgType(IeId id); // override
        
    };

    //------------------------------------------------------------------------------

    class PacketConnect : public PacketType<L3MessgeType::CONNECT>
    {
        typedef PacketType<L3MessgeType::CONNECT> Base;
    public:

          PacketConnect(IsdnInfra& infra, IeFactory& ieFactory, QVector<byte> l2data );

          PacketConnect(IIsdnL3Internal& IL3, const CallRef &callref);

    private:

          bool ValidateMandatoryIes(const std::multiset<IeId>& exist) const; // override

          void UpRout(L3Call &);  // override

          bool IeCorrectForThisMsgType(IeId id); // override;
    };

    //------------------------------------------------------------------------------

    
    class PacketConnectAck : public PacketType<L3MessgeType::CONNECT_ACKNOWLEDGE>
    {
        typedef PacketType<L3MessgeType::CONNECT_ACKNOWLEDGE> Base;
    public:

        PacketConnectAck(IsdnInfra& infra, IeFactory& ieFactory, QVector<byte> l2data );

        PacketConnectAck(IIsdnL3Internal& IL3, const CallRef &callref);

    private:

        bool ValidateMandatoryIes(const std::multiset<IeId>& exist) const; // override

		void UpRout(L3Call &);  // override

        bool IeCorrectForThisMsgType(IeId id); // override
    };

    
    //------------------------------------------------------------------------------

    class PacketDisc : public PacketType<L3MessgeType::DISCONNECT>
    {
        typedef PacketType<L3MessgeType::DISCONNECT> Base;
    public:

        PacketDisc(IsdnInfra& infra, IeFactory& ieFactory, QVector<byte> l2data );

        PacketDisc(IIsdnL3Internal& IL3, const CallRef &callref);

    private:

        bool ValidateMandatoryIes(const std::multiset<IeId>& exist) const;

		void UpRout(L3Call &);  // override

        bool IeCorrectForThisMsgType(IeId id); // override
            
    };


    //------------------------------------------------------------------------------

    class PacketSetup : public PacketType<L3MessgeType::SETUP>
    {
        typedef PacketType<L3MessgeType::SETUP> Base;
    public:

        PacketSetup(IsdnInfra& infra, IeFactory& ieFactory, QVector<byte> l2data );

        PacketSetup(IIsdnL3Internal& IL3, const CallRef &callref);
      
    private:

        bool ValidateMandatoryIes(const std::multiset<IeId>& exist) const; // override

		void UpRout(L3Call &);  // override

        bool IeCorrectForThisMsgType(IeId id); // override
        
    };

    //------------------------------------------------------------------------------

    class PacketSetupAck : public PacketType<L3MessgeType::SETUP_ACKNOWLEDGE>
    {
        typedef PacketType<L3MessgeType::SETUP_ACKNOWLEDGE> Base;
    public:

          PacketSetupAck(IsdnInfra& infra, IeFactory& ieFactory, QVector<byte> l2data );

          PacketSetupAck(IIsdnL3Internal& IL3, const CallRef &callref);

    private:

        bool ValidateMandatoryIes(const std::multiset<IeId>& exist) const ;// override

		void UpRout(L3Call &);  // override

        bool IeCorrectForThisMsgType(IeId id); 
    };


    //------------------------------------------------------------------------------

    class PacketProceeding : public PacketType<L3MessgeType::CALL_PROCEEDING>
    {
        typedef PacketType<L3MessgeType::CALL_PROCEEDING> Base;
    public:

        PacketProceeding(IsdnInfra& infra, IeFactory& ieFactory, QVector<byte> l2data );

        PacketProceeding(IIsdnL3Internal& IL3, const CallRef &callref);

    private:

        bool ValidateMandatoryIes(const std::multiset<IeId>& exist) const; // override

		void UpRout(L3Call &);  // override

        bool IeCorrectForThisMsgType(IeId id); // override
    };

    //------------------------------------------------------------------------------

    class PacketRelease : public PacketType<L3MessgeType::RELEASE>
    {
        typedef PacketType<L3MessgeType::RELEASE> Base;
    public:

        PacketRelease(IsdnInfra& infra, IeFactory& ieFactory, QVector<byte> l2data );

        PacketRelease(IIsdnL3Internal& IL3, const CallRef &callref);

    private:

        bool ValidateMandatoryIes(const std::multiset<IeId>& exist) const; // override

		void UpRout(L3Call &);  // override

        bool IeCorrectForThisMsgType(IeId id); // override
    };

    //------------------------------------------------------------------------------

    class PacketReleaseComplete : public PacketType<L3MessgeType::RELEASE_COMPLETE>
    {
        typedef PacketType<L3MessgeType::RELEASE_COMPLETE> Base;
    public:

        PacketReleaseComplete(IsdnInfra& infra, IeFactory& ieFactory, QVector<byte> l2data );        

        PacketReleaseComplete(IIsdnL3Internal& IL3, const CallRef &callref);

    private:

        bool ValidateMandatoryIes(const std::multiset<IeId>& exist) const; // override

		void UpRout(L3Call &);  // override

        // вызывыается при добавлении Ие
        bool IeCorrectForThisMsgType(IeId id); // override

    };

    //------------------------------------------------------------------------------

    class PacketStatus : public PacketType<L3MessgeType::STATUS>
    {
        typedef PacketType<L3MessgeType::STATUS> Base;
    public:

        PacketStatus(IsdnInfra& infra, IeFactory& ieFactory, QVector<byte> l2data );

        PacketStatus(IIsdnL3Internal& IL3, const CallRef &callref);

    private:

        bool ValidateMandatoryIes(const std::multiset<IeId>& exist) const; // override

		void UpRout(L3Call &);  // override

        bool IsGlobalRouting() const {return true;}
        void UpRout(IsdnL3& receiver); 

        // вызывыается при добавлении Ие
        bool IeCorrectForThisMsgType(IeId id); // override

    };

    //------------------------------------------------------------------------------

    class PacketInfo : public PacketType<L3MessgeType::INFORMATION>
    {
        typedef PacketType<L3MessgeType::INFORMATION> Base;

    public:

        PacketInfo(IsdnInfra& infra, IeFactory& ieFactory, QVector<byte> l2data );

        PacketInfo(IIsdnL3Internal& IL3, const CallRef &callref);

    private:

        bool ValidateMandatoryIes(const std::multiset<IeId>& exist) const; // override

		void UpRout(L3Call &);  // override

        // вызывыается при добавлении Ие
        bool IeCorrectForThisMsgType(IeId id); // override
    };

    //------------------------------------------------------------------------------

    class PacketProgress : public PacketType<L3MessgeType::PROGRESS>
    {
        typedef PacketType<L3MessgeType::PROGRESS> Base;

    public:

        PacketProgress(IsdnInfra& infra, IeFactory& ieFactory, QVector<byte> l2data );

        PacketProgress(IIsdnL3Internal& IL3, const CallRef &callref);

    // override:
    private:

        bool ValidateMandatoryIes(const std::multiset<IeId>& exist) const;

		void UpRout(L3Call &);  // override

        // вызывыается при добавлении Ие
        bool IeCorrectForThisMsgType(IeId id);
    };

    //------------------------------------------------------------------------------

    class PacketRestart : public PacketType<L3MessgeType::RESTART>
    {
        typedef PacketType<L3MessgeType::RESTART> Base;

    public:

        PacketRestart(IsdnInfra& infra, IeFactory& ieFactory, QVector<byte> l2data );

        PacketRestart(IIsdnL3Internal& IL3/*, const CallRef &callref*/);

        // override:
    private:

        bool ValidateMandatoryIes(const std::multiset<IeId>& exist) const;

        void UpRout(L3Call &);  // override;

        // вызывыается при добавлении Ие
        bool IeCorrectForThisMsgType(IeId id);

        bool IsGlobalRouting() const {return true;}
        void UpRout(IsdnL3& receiver);        
    };

    // ------------------------------------------------------------------------------------

    class PacketRestartAck : public PacketType<L3MessgeType::RESTART_ACKNOWLEDGE>
    {
        typedef PacketType<L3MessgeType::RESTART_ACKNOWLEDGE> Base;

    public:

        PacketRestartAck(IsdnInfra& infra, IeFactory& ieFactory, QVector<byte> l2data );

        PacketRestartAck(IIsdnL3Internal& IL3/*, const CallRef &callref*/);

        // override:
    private:

        bool ValidateMandatoryIes(const std::multiset<IeId>& exist) const;

        void UpRout(L3Call &);  // override;

        // вызывыается при добавлении Ие
        bool IeCorrectForThisMsgType(IeId id);

        bool IsGlobalRouting() const {return true;}
        void UpRout(IsdnL3& receiver); 
    };

    // ------------------------------------------------------------------------------------


    static void RegL3Packets(L3PacketFactory& factory)
    {
        factory.RegPack<PacketAlerting>();
        factory.RegPack<PacketConnect>();
        factory.RegPack<PacketConnectAck>();
        factory.RegPack<PacketSetup>();
        factory.RegPack<PacketSetupAck>();
        factory.RegPack<PacketProceeding>();
        factory.RegPack<PacketDisc>();
        factory.RegPack<PacketRelease>();
        factory.RegPack<PacketReleaseComplete>();
        factory.RegPack<PacketStatus>();
        factory.RegPack<PacketInfo>();
        factory.RegPack<PacketProgress>();
        factory.RegPack<PacketRestart>();
        factory.RegPack<PacketRestartAck>();
        //factory.RegPack<>();
    }

} // namespace ISDN

#endif

