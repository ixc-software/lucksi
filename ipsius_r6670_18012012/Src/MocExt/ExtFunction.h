#ifndef __EXTPFUNCTION__

#define  __EXTPFUNCTION__

#include "moc.h"

// cross reference, must be defined later in moc.h
struct FunctionDef;

namespace MocExt
{
    // ---------------------------------------------------------------
	// ������ �������� Function ��� ������� ���������� ��������� Proxy	
    class FunctionList
    {
    public:        

        void operator += (const QList<FunctionDef> &functionList);
        
        bool checkFunctions() const;
        void GenerateCrc(QByteArray &funcCRC) const;						        
        void GenerateEnum(QTextStream &funcOut) const;						
        void GenerateLocalProxyOld(QTextStream &out) const;
        void GenerateLocalProxyNew(QTextStream &out) const;		
        void GenerateRemoteProxy(QTextStream &out) const;
        void GenerateServerProxy(QTextStream &out) const;
        
    private:
        // ����� ��������� ���� ��� ������ ������ 
        class Function {
        public:

            Function(const FunctionDef &functionDef)            
            {
                this->m_FunctionDef = functionDef;
                GenerateSignature();
            }
            
            bool checkFunctions() const;
           
            void Generate(QTextStream &funcOut, QByteArray &funcCRC) const;
            void GenerateEnumIdMethod(QTextStream &out) const;
            void GenerateEnumIdCrc(QTextStream &out) const;
            void GenerateEnumCrc(QTextStream &out) const;
            void GenerateCrcStr(QByteArray &funcCRC) const;
            void GenerateLocalProxyOld(QTextStream &out) const;
            void GenerateLocalProxyNew(QTextStream &out) const;			
            void GenerateRemoteProxy(QTextStream &out) const;
            void GenerateServerProxy(QTextStream &out) const;
        
        private:
            void GenerateSignature();
            QByteArray m_Signature;
            QByteArray m_onSignature;
            QByteArray m_Arguments;
            FunctionDef m_FunctionDef;

        };

        // end of ����� ��������� ���� ��� ������        
        QList<FunctionDef> m_FunctionsList;

        // ���� ������� ����������� � ������, ��������� �� 
        void Append(const FunctionDef &functionDef);
    };// class FunctionList
	
} // namespace MocExt   

#endif

