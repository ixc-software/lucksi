#include "ExtOptions.h"
#include "ExtProperty.h"

namespace 
{
	bool IsDirectProperty(const QByteArray &arr)
	{
		if(!MocExt::ExtOptionsEnabled()) return false;

		return arr.startsWith("m_") || arr.startsWith("(m_");
	}
};

namespace MocExt
{
    bool PrintReadProperty(FILE *out, ClassDef *cdef, int propindex, const PropertyDef &p)
    {
		if(!IsDirectProperty(p.read))  return false;

        if (p.gspec == PropertyDef::PointerSpec)
            fprintf(out, "        case %d: _a[0] = const_cast<void*>(reinterpret_cast<const void*>(%s)); break;\n",
            propindex, p.read.constData());
        else if (p.gspec == PropertyDef::ReferenceSpec)
            fprintf(out, "        case %d: _a[0] = const_cast<void*>(reinterpret_cast<const void*>(&%s)); break;\n",
            propindex, p.read.constData());
        else if (cdef->enumDeclarations.value(p.type, false))
            fprintf(out, "        case %d: *reinterpret_cast<int*>(_v) = QFlag(%s); break;\n",
            propindex, p.read.constData());
        else
            fprintf(out, "        case %d: *reinterpret_cast< %s*>(_v) = %s; break;\n",
				propindex, p.type.constData(), p.read.constData());
		
		return true;
    }

    bool PrintWriteProperty(FILE *out, ClassDef *cdef, int propindex, const PropertyDef &p)
    {
		if(!IsDirectProperty(p.write))  return false;

		if(!p.write.endsWith("())"))
		{
			// direct nested property
			if (cdef->enumDeclarations.value(p.type, false)) {
				fprintf(out, "        case %d: %s = QFlag(*reinterpret_cast<int*>(_v)); break;\n",
					propindex, p.write.constData());
			} else {
				fprintf(out, "        case %d: %s = *reinterpret_cast< %s*>(_v); break;\n",
					propindex, p.write.constData(), p.type.constData());
			}
			return true;
		}
		// nested method
		QByteArray write(p.write);

		write.remove(write.size() - 3, 2);// paren deleted (m_property.SomeFunction()) -> (m_property.SomeFunction)

		if (cdef->enumDeclarations.value(p.type, false)) {
			fprintf(out, "        case %d: %s(QFlag(*reinterpret_cast<int*>(_v))); break;\n",
				propindex, write.constData());
		} else {
			fprintf(out, "        case %d: %s(*reinterpret_cast< %s*>(_v)); break;\n",
				propindex, write.constData(), p.type.constData());
		}
        return true;
    }

};


