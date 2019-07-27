--Kayas Ahmed


library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values

use IEEE.NUMERIC_STD.ALL;

-- Uncomment the following library declaration if instantiating
-- any Xilinx leaf cells in this code.
library UNISIM;
use UNISIM.VComponents.all;

entity dht_top is
    Port ( data_in      : inout STD_LOGIC;
           clk          : in STD_LOGIC;
           device_enable           : in std_logic;
           temp         : out std_logic_vector(7 downto 0);
           end_p :out std_logic;
           humidity     : out std_logic_vector(7 downto 0);
           debug        : out std_logic_vector(31 downto 0);
           crc           :out std_logic_vector(7 downto 0);
           rst          : in std_logic);
end dht_top;

architecture Behavioral of dht_top is

constant Delay_18_ms :positive := 18 * 10**6 / 20 + 1;
constant Delay_30_us :positive := 30 * 10**3 / 20 + 1;
constant Delay_50_us :positive := 50 * 10**3 / 20 + 1;
constant Delay_75_us :positive := 75 * 10**3 / 20 + 1;
constant Delay_80_us :positive := 80 * 10**3 / 20 + 1;
constant Delay_100_ms :positive := 100 * 10**6 / 20 + 1;
constant Delay_1s:positive := 1000 * 10**6 / 20 + 1;
signal input_sync   :std_logic_vector(1 downto 0);
type state is (Init,Idle,Request,Response,Response_1,Response_2,Data,Data_1,End_process);
signal rising_sig,falling_sig :boolean;
signal curr_state,next_state :state; 
signal rst_counter :std_logic;
signal count: integer ;
signal en: std_logic ;
signal data_s: std_logic;
signal data_count: integer range 0 to 40;
signal data_word: std_logic_vector(39 downto 0);
signal dec:std_logic;
signal set_one,set_zero:std_logic;
begin

--signal edge detector
edge_detect:process(clk,rst)
 begin  
    if clk'event and clk='1' then
        if rst='0' then
            input_sync<="00";
        else
       
        input_sync<=input_sync(0)&data_s;
        end if;
    end if;
 end process;
rising_sig  <= input_sync(1 downto 0) = "01";
falling_sig <= input_sync(1 downto 0) = "10";


counter: process(clk)
 begin
    if rising_edge(clk) then
        if rst='0' then
            count<=0;
             data_word<=(others=>'0');
        else
            if rst_counter='1' then
                count<=0;
            else
                count<=count+1;
            end if;
        end if;
        
        if dec='1' then
            data_count<=data_count-1;
        end if;
        if curr_state=Request then
            data_count<=40;
             data_word<=(others=>'0');
        end if;
        if set_one='1' then
            data_word(data_count-1)<='1';
            data_count<=data_count-1;
        end if;
        if set_zero='1' then
            data_word(data_count-1)<='0';
            data_count<=data_count-1;
        end if;
        
        if curr_state=End_process then
            temp<=data_word((40-(8*2))-1 downto (40-(8*3)));
            humidity<=data_word((40-(8*0))-1 downto (40-(8*1)));
            debug<=data_word(39 downto 8);
            crc<=data_word(7 downto 0);
        end if;
    end if;
          
            
 end process;

clk_process:process(clk,rst)
begin
  if rising_edge(clk) then
    if rst='0' then
        curr_state<=Init;
    else
        curr_state<=next_state;
    end if;
 end if;
end process;

fsm:process(curr_state,count,falling_sig,rising_sig,data_count)
 begin
  
   
       next_state<=curr_state; 
       rst_counter<='0';
       en<='1';
       dec<='0';
       set_one<='0';
       set_zero<='0'; 
       end_p <='0';
   
    case curr_state is
        when Init =>
            if device_enable='1' then
                rst_counter<='1'; 
                next_state<=Idle;
           
            end if;
        when Idle =>
            en<='0';
            if count=Delay_18_ms then
                next_state<=Request;
                rst_counter<='1';
             end if;
         
        when Request =>
              next_state<=Response;
        when Response =>
            if falling_sig then
                next_state<=Response_1;
                rst_counter<='1';     
            end if;
            if count>Delay_100_ms then
                next_state<=Init;
            end if;
         
        when Response_1 =>
            if rising_sig then
                next_state<=Response_2;
                rst_counter<='1'; 
            end if;
            if count>Delay_100_ms then
                next_state<=Init;
            end if;
        when Response_2 =>
            if falling_sig then
                next_state<=Data;
                rst_counter<='1'; 
            end if;
           
            if count>Delay_100_ms then
                next_state<=Init;
            end if;
        when Data =>
            if rising_sig then
                next_state<=Data_1;
                rst_counter<='1'; 
            end if;
            if data_count=0 then
                rst_counter<='1';
                next_state<=End_process;
            end if;
            
            if count>Delay_100_ms then
                next_state<=Init;
            end if;
        when Data_1  =>
            if  falling_sig then
                if count<= Delay_30_us then
                    set_zero<='1';
                elsif count<= Delay_75_us then
                    set_one<='1';
                else
                end if;
                next_state<=Data; 
            end if;
            
            if count>Delay_100_ms then
               next_state<=Init;
            end if;
         when End_process =>
            if count=Delay_1s then
                next_state<=Init;
                rst_counter<='1';
                end_p <='1';
            end if;
          
        when others =>
         
    end case;
 
 end process;
 
--concurrent statement

 u1: iobuf
 generic map(
 drive =>12,
 iostandard =>"lvcmos33",
 slew=>"slow")
  port map(
 o=>data_s,
 io=>data_in,
 i=>'0',
 t=>en);





 end Behavioral;
