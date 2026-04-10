module Components {
    @ Processes images into star vectors and satellite orientation
    passive component StarTracker {
        

        ##############################################################################
        #### Uncomment the following examples to start customizing your component ####
        ##############################################################################

        # Input image port: Receives raw image data from the camera. The buffer contains pixel data that the star tracker will process to detect stars and compute centroids.
        sync input port getPicture: LionessSw.GetPicture

  
        ###############################################################################
        # Telemetry #
        ###############################################################################
        # @ Tells us how many stars are currently being tracked
        # Too high = Noise ,  Too low = Threshold  too strict, 0 = Something is wrong
        telemetry NumStars: U32

        ###############################################################################
        # Events #
        ###############################################################################

        

        ###############################################################################
        # Parameters #
        ###############################################################################
        
        param FocalLengthMM: F64 default 49

        param PixelSizeUM: F64 default 22.2

        param MinMagnitude: U32 default 5
        
        # 5 standard deviations above the  mean of the entire image
        param ThresholdMultiplier: F32 default 5.0

        param MaxStars: U16 default 20

        param ToleranceDeg: F64 default 0.05

        param NumFalseStars: U32 default 1000

        param MaxMismatchProbability: F64 default 0.0001

        param Cutoff: U64 default 1000

        
        # @ Example event
        # event ExampleStateEvent(example_state: Fw.On) severity activity high id 0 format "State set to {}"

        # @ Example port: receiving calls from the rate group
        # sync input port run: Svc.Sched

        # @ Example parameter
        # param PARAMETER_NAME: U32

        ###############################################################################
        # Standard AC Ports: Required for Channels, Events, Commands, and Parameters  #
        ###############################################################################
        @ Port for requesting the current time
        time get port timeCaller

        @ Enables command handling
        import Fw.Command

        @ Enables event handling
        import Fw.Event

        @ Enables telemetry channels handling
        import Fw.Channel

        @ Port to return the value of a parameter
        param get port prmGetOut

        @Port to set the value of a parameter
        param set port prmSetOut

    }
}