/*
 *	SetAttributeCommand.h
 *	!CHOAS
 *	Created by Bisegni Claudio.
 *
 *    	Copyright 2013 INFN, National Institute of Nuclear Physics
 *
 *    	Licensed under the Apache License, Version 2.0 (the "License");
 *    	you may not use this file except in compliance with the License.
 *    	You may obtain a copy of the License at
 *
 *    	http://www.apache.org/licenses/LICENSE-2.0
 *
 *    	Unless required by applicable law or agreed to in writing, software
 *    	distributed under the License is distributed on an "AS IS" BASIS,
 *    	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    	See the License for the specific language governing permissions and
 *    	limitations under the License.
 */

#ifndef __CHAOSFramework__SetAttributeCommand__
#define __CHAOSFramework__SetAttributeCommand__

#include <chaos/cu_toolkit/ControlManager/slow_command/SlowCommand.h>

namespace chaos {
    namespace cu {
        namespace control_manager {
            namespace slow_command {
                
                namespace command {
                    
                    //! Set Property Command
                    /*!
                      This command permit to update an attribute into the shared setting instance
                     of the sandbox.
                     */
                    class SetAttributeCommand : public SlowCommand {
                    protected:
                        // return the implemented handler
                        uint8_t implementedHandler();
                        
                        // Start the command execution
                        void setHandler(chaos::common::data::CDataWrapper *data);
                    };
                    
                }
            }
        }
    }
}

#endif /* defined(__CHAOSFramework__SetAttributeCommand__) */
