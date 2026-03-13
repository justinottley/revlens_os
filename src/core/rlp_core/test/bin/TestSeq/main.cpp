
#include "RlpCore/LOG/Logging.h"

#include "RlpCore/SEQ/Util.h"
#include "RlpCore/SEQ/Sequence.h"

int
main(int argc, char** argv)
{
    LOG_Logging::init();

    QString path = argv[1];

    SequenceList seqList = SEQ_Util::getSequences(path);

    for (int si=0; si != seqList.size(); ++si)
    {
        SEQ_SequencePtr seq = seqList.at(si);
        qInfo() << si << " size:" << seq->size();

        for (int ii = 0; ii != seq->size(); ++ii)
        {
            qInfo() << seq->at(ii)->signature()->toString();
        }
    }


    return 0;
}