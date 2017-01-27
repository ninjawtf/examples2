		private async Task startProcessMonitoring()
		{
			try
			{
				_process = getProcess(_cmdPattern);

				var counter = createCounter();

				var enumerator = counter.GetDataStringRepresentation().GetEnumerator();

				// скипаем первый элемент, т.к. у него нулевой интервал
				if (!enumerator.MoveNext())
					return;

				await Task.Delay(TimeSpan.FromSeconds(_updateInterval), _cts.Token).ConfigureAwait(false);

				while (enumerator.MoveNext() && !_cts.IsCancellationRequested)
				{
					var newElement = new DataElement(_zabbixItemKey, enumerator.Current);

					lock (_padlock)
					{
						_dataToSend.Add(newElement);
					}

					await Task.Delay(TimeSpan.FromSeconds(_updateInterval), _cts.Token).ConfigureAwait(false);
				}
			}
			catch (Exception e)
			{
				await Task.Delay(TimeSpan.FromSeconds(_discoveryDropedProcessInterval), _cts.Token).ConfigureAwait(false);

				_logger.Warn("Monitor error. Item {0}, error: {1}. Trying to restart process", _zabbixItemKey, e);
				await startProcessMonitoring();
			}
		}
