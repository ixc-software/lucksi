#ifndef IMGDESCRLIST_H
#define IMGDESCRLIST_H

#include "ImgDescriptor.h"

namespace BfBootCore
{
    class ImgDescrList
    {
        std::vector<ImgDescriptor> m_list;

    public:
        int Size() const;
        const ImgDescriptor& operator[](int i) const;
        void Add(const ImgDescriptor& descr);
        const ImgDescriptor& Back() const;
        void DeleteLast();
        void Clear();

        std::string ToString() const; // readable string

        // for test
        bool operator ==(const ImgDescrList& rhs) const;
        bool operator !=(const ImgDescrList& rhs) const;

    // Используется StringToTypeConverter при передаче 
    // через протокол бутера и при сериализации в Config.
    private:        
        friend class StringToTypeConverter;
        void SetFrom(std::vector<ImgDescriptor> &list);
        const std::vector<ImgDescriptor>& AsVector() const;

    };
} // namespace BfBootCore

#endif
