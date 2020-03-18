import java.util.concurrent.atomic.AtomicBoolean;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.logging.ConsoleHandler;
import java.util.logging.Logger;

public class Application {
    public Application() {
    }
    
    protected static AtomicBoolean shouldRun;
    protected static Logger logger;

    static class ApplicationArguments {
        public static AtomicInteger domainId = new AtomicInteger(0);
        public static AtomicInteger sampleCount = new AtomicInteger(0);
        public static AtomicInteger verbosity  = new AtomicInteger(0);
        public static AtomicBoolean runApplication  = new AtomicBoolean(true);
    }

    public static void parseArguments(
            String[] args) throws Exception {
        
        int argProcessing = 0;
        boolean showUsage = false;
        ParseReturn parseResult = ParseReturn.PARSE_RETURN_SUCCESS;
        
        while (argProcessing < args.length) {
            if (args[argProcessing] == "-d"
                    || args[argProcessing] == "--domain") {
                ApplicationArguments.domainId.set(
                        Integer.parseInt(args[argProcessing + 1]));
                argProcessing += 2;
            } else if (args[argProcessing] == "-s"
                    || args[argProcessing] == "--sample-count") {
                ApplicationArguments.sampleCount.set(
                        Integer.parseInt(args[argProcessing + 1]));
                argProcessing += 2;
            } else if (args[argProcessing] == "-v"
                    || args[argProcessing] == "--verbosity") {
                ApplicationArguments.verbosity.set(
                        Integer.parseInt(args[argProcessing + 1]));
                argProcessing += 2;
            } else if (args[argProcessing] == "-h"
                    || args[argProcessing] == "--help") {
                logger.info("Example application.");
                showUsage = true;
                parseResult = ParseReturn.PARSE_RETURN_EXIT;
                break;
            } else {
                logger.severe("Bad parameter.");
                showUsage = true;
                parseResult = ParseReturn.PARSE_RETURN_FAILURE;
                break;
            }
        }
        if (showUsage) {
            logger.info("Usage:\n" +
                        "    -d, --domain       <int>   Domain ID this application will\n" +
                        "                               subscribe in.  \n" +
                        "                               Default: 0\n" +
                        "    -s, --sample_count <int>   Number of samples to receive before\n" +
                        "                               cleanly shutting down. \n" +
                        "                               Default: infinite\n" +
                        "    -v, --verbosity    <int>   How much debugging output to show.\n" +
                        "                               Range: 0-5 \n" +
                        "                               Default: 0");
        }
        
        if (parseResult == ParseReturn.PARSE_RETURN_FAILURE) {
            throw new Exception("Bad parameter passed to applicaiton");
        } else {
            ApplicationArguments.runApplication.set(true);
        }
    }
    
    public static void handleShutdown() {
        shouldRun = new AtomicBoolean(true);
        
        Runtime.getRuntime().addShutdownHook(new Thread() {
            public void run() {
                logger.info("Shutting down...");
                shouldRun.set(false);
            }
        });

    }
    
    public static void setUpLogging() {
        logger = Logger.getLogger(HelloMessagePublisher.class.getName());
        logger.setUseParentHandlers(false);
        logger.addHandler(new ConsoleHandler() {
            {setOutputStream(System.out);}
        });

    }
        
    enum ParseReturn {
        PARSE_RETURN_FAILURE,
        PARSE_RETURN_SUCCESS,
        PARSE_RETURN_EXIT
    }

}
