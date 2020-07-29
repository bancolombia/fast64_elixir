defmodule Fast64 do
  @moduledoc """
  Documentation for `Fast64`.
  """

  @doc """
  Fast64

  ## Examples

      iex> Fast64.encode64("hello")
      "aGVsbG8="

      iex> Fast64.decode64("aGVsbG8=")
      "hello"

  """
  @on_load :load_nif

  app = Mix.Project.config[:app]
  def load_nif do
    path = :filename.join(:code.priv_dir(unquote(app)), 'fast64')
    :ok = :erlang.load_nif(path, 0)
  end

  def decode64(_data) do
    raise "NIF Fast64.decode64/1 not implemented"
  end

  def encode64(_data) do
    raise "NIF Fast64.encode64/1 not implemented"
  end

end
