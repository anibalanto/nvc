entity vhpi1 is
    port (
        x : in natural;
        y : out natural );
end entity;

architecture test of vhpi1 is
    signal v : bit_vector(3 downto 0) := "0011";
    signal b : bit;
begin

    process (x) is
    begin
        report "x=" & integer'image(x);
        y <= x + 1 after 1 ns;
    end process;

end architecture;
