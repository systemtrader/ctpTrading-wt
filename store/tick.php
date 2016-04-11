<?php
require 'consumer.php';
/**
*
*/
class Tick
{

    function __construct()
    {
        $this->consumer = new Consumer();
    }

    public function run()
    {
        while (true) {
            $data = $this->consumer->popViaRds("MARKET_TICK_Q");
            $localTime = $data[0];
            sleep(1);
        }
    }
}

(new Tick)->run();
