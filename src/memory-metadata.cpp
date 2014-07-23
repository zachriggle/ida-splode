/*
 * MemoryMetadata.cpp
 *
 *  Created on: Oct 22, 2013
 */

#include "common.h"

set<MetadataProvider *> MetadataProvider::Instances;





//---------------- STACK FRAME METADATA PROVIDER :: CONSTRUCTOR ----------------
StackFrameMetadataProvider::StackFrameMetadataProvider
(
)   : SortableMixin(-2)
{}

//------------------ STACK FRAME METADATA PROVIDER :: ENHANCE ------------------
bool
StackFrameMetadataProvider::Enhance
(
    ostream   & os,
    ADDRINT     Val,
    ADDRINT     Size
) const
{
    const CallFrame * Frame = Thread::FindFrameContaining(Val);

    if (NULL != Frame)
    {
        Symbol *sym = Symbol::ResolveSymbol(Frame->FunctionEntry);

        os  << "\"Stack\": { \"Entry\": ";

        if (sym)
        {
            os << (*sym);
        }
        else
        {
            os << Frame->FunctionEntry;
        }

        os  << ", \"Offset\":" << Frame->OffsetFromRa(Val)
            << "},";

        return true;
    }

    return false;
}


//-------------- HEAP ALLOCATION METADATA PROVIDER :: CONSTRUCTOR --------------
HeapAllocationMetadataProvider::HeapAllocationMetadataProvider
(
)   : SortableMixin(2)
{}
//---------------- HEAP ALLOCATION METADATA PROVIDER :: ENHANCE ----------------
bool
HeapAllocationMetadataProvider::Enhance
(
    ostream   & os,
    ADDRINT     Val,
    ADDRINT     Size
) const
{
    HeapStackTrace HeapTrace((void *) Val);

    if (HeapTrace.PageHeapBlock)
    {
        os << "\"Heap\": " << HeapTrace << ", ";
        return true;
    }

    return false;
}

//------------------ SYMBOL METADATA PROVIDER :: CONSTRUCTOR -------------------
SymbolMetadataProvider::SymbolMetadataProvider
(
)   : SortableMixin(0)
{}

//-------------------- SYMBOL METADATA PROVIDER :: ENHANCE ---------------------
bool
SymbolMetadataProvider::Enhance
(
    ostream   & os,
    ADDRINT     Val,
    ADDRINT     Size
) const
{
    PIN_LockClient();
    bool IsImage = IMG_Valid(IMG_FindByAddress(Val));
    PIN_UnlockClient();

    if (IsImage && !IsAddressWhitelisted(Val))
    {
        Symbol *sym = Symbol::ResolveSymbol(Val);

        if (sym)
        {
            os << "\"Symbol\": " << *sym << ", ";
        }

        return true;
    }

    return false;
}


//---------------------- METADATA PROVIDER :: CONSTRUCTOR ----------------------
MetadataProvider::MetadataProvider
(
)
{
    Instances.insert(this);
}


//---------------------- METADATA PROVIDER :: DESTRUCTOR -----------------------
MetadataProvider::~MetadataProvider
(
)
{
    Instances.erase(this);
}


//-------------- HEAP ALLOCATION METADATA PROVIDER :: CONSTRUCTOR --------------
TextualMetadataProvider::TextualMetadataProvider
(
)   : SortableMixin(-3)
{}


//---------------- HEAP ALLOCATION METADATA PROVIDER :: ENHANCE ----------------
template<typename T>
string escapeJSON(const T* input);

bool
TextualMetadataProvider::Enhance
(ostream& os, ADDRINT Value, ADDRINT Size) const
{
    const size_t NumChars = 0x20;
    union
    {
        char    asciiBuffer[NumChars];
        wchar_t utfBuffer[NumChars];
    } buf;

    size_t BytesCopied = PIN_SafeCopy(&buf, (void*) Value, sizeof(buf));

    if (IsProbablyUTF16(buf.utfBuffer, BytesCopied))
    {

        buf.utfBuffer[NumChars - 1] = 0;
        os << ", \"Text\": \"" << escapeJSON(buf.utfBuffer) << "...\"";
    }
    else if (IsProbablyAscii(buf.asciiBuffer, BytesCopied))
    {

        buf.asciiBuffer[NumChars - 1] = 0;
        os << ", 'Text': \"" << escapeJSON(buf.asciiBuffer) << "...\"";
    }

    return false;
}

template<typename T>
string escapeJSON(const T* input) {
    ostringstream ss;

        ss << "Foo bar baz";

    while (1) {
        switch (*input++) {
            case '\\': ss << "\\\\"; break;
            case '"': ss << "\\\""; break;
            case '/': ss << "\\/"; break;
            case '\b': ss << "\\b"; break;
            case '\f': ss << "\\f"; break;
            case '\n': ss << "\\n"; break;
            case '\r': ss << "\\r"; break;
            case '\t': ss << "\\t"; break;
            case '\0': return ss.str();
            default: ss << *input; break;
        }
    }
    return "";
}

//------------------ ADDRESS METADATA :: OSTREAM OPERATOR << -------------------
ostream&
operator<<
(
    ostream               & os,
    const AddressMetadata & mm
)
{

    bool IsMemory = IsProbablyPointer(mm.Value) && PIN_CheckReadAccess((void *)mm.Value);

    os  << "{\"Value\":" << mm.Value
        << ",\"Size\":" << mm.Size
        << ",\"Memory\":" << IsMemory
        << ",";

    for (auto i = MetadataProvider::Instances.begin();
         i != MetadataProvider::Instances.end()
      && IsMemory
      && false == (*i)->Enhance(os, mm.Value, mm.Size);
         i++)
    {}

    return os << "}";
}
