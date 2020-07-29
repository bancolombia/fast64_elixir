defmodule Fast64Test do
  use ExUnit.Case
  doctest Fast64

  test "should encode" do
    assert Fast64.encode64("hello") == "aGVsbG8="
  end

  test "should decode" do
    assert Fast64.decode64("aGVsbG8=") == "hello"
  end

  test "should decode empty" do
    assert Fast64.decode64("") == ""
  end

  test "should encode empty" do
    assert Fast64.encode64("") == ""
  end

  test "should raise on encode nil" do
    assert_raise ArgumentError, fn ->
      Fast64.encode64(nil)
    end
  end

  test "should decode ignore space" do
    assert Fast64.decode64("Zm9vYmFy\n") == "foobar"
    assert Fast64.decode64("Zm9vYmFy ") == "foobar"
  end

  test "should decode ignore padding" do
    assert Fast64.decode64("Zm9vYg==") == "foob"
    assert Fast64.decode64("Zm9vYg") == "foob"
  end

  test "should raise on encode number" do
    assert_raise ArgumentError, fn ->
      Fast64.encode64(121)
    end
  end

  test "should raise on decode number" do
    assert_raise ArgumentError, fn ->
      Fast64.decode64(121)
    end
  end

  test "should raise on decode nil" do
    assert_raise ArgumentError, fn ->
      Fast64.decode64(nil)
    end
  end

  test "should decode no pad" do
    assert Fast64.decode64("aGVsbG8") == "hello"
  end

  test "should decode performance" do
    data = :binary.copy("0123456789", 1000)
    data2 = :binary.copy("0123456789", 10)
    encoded = Fast64.encode64(data)
    _ = Base.encode64(data)
    Fast64.decode64(encoded)
    Base.decode64(encoded)
    {time_encode, encoded2} = :timer.tc(fn -> Fast64.encode64(data2) end)
    {time_decode, _} = :timer.tc(fn -> Fast64.decode64(encoded2) end)
    IO.inspect({time_encode, time_decode})

    {time_encode2, _} = :timer.tc(fn -> Base.encode64(data2) end)
    {time_decode2, _} = :timer.tc(fn -> Base.decode64(encoded2) end)
    IO.inspect({time_encode2, time_decode2})
    IO.inspect({time_encode2/time_encode, time_decode2/time_decode})

  end

end
