#ifndef _EDS_PSX_BUTTON_H_
#define _EDS_PSX_BUTTON_H_

#include "EventDataSource.h"

/*
*   Event Data Source for PSX Button
*
*       Get Data: send "A8 8A 02 01 03 ED" to bus 
*                 wait for return "A8 8A 0B 01 -- -- -- ?? ?? -- -- -- -- -- ED" 
*/

class EdsPsxButton : public EventDataSource {

    public:
        EdsPsxButton(Stream *busPort, Stream *debugPort);
        ~EdsPsxButton();

        void Initialize(EventData *data) override;
        void GetData() override;
        void PostHandler() override;

    private:
        Stream *_bus = NULL;

};

#endif