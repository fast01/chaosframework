package it.infn.chaos.mds.batchexecution;

import java.util.Hashtable;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.TimeUnit;

public class SlowExecution {
	private ScheduledExecutorService								executionService	= null;
	private Hashtable<String, Class<? extends SlowExecutionJob>>	mapAliasJob			= new Hashtable<String, Class<? extends SlowExecutionJob>>();

	public SlowExecution(int threadNumber) {
		executionService = Executors.newScheduledThreadPool(threadNumber);
	}

	public void registerSlowControlJob(Class<? extends SlowExecutionJob> job) {
		mapAliasJob.put(job.getName(), job);
	}

	public boolean submitJob(String alias, Object jobData) throws InstantiationException, IllegalAccessException {
		if (!mapAliasJob.containsKey(alias))
			return false;
		SlowExecutionJob job = mapAliasJob.get(alias).newInstance();
		job.setInputData(jobData);
		executionService.execute(job);
		return true;
	}

	public void submitJob(String alias, Object jobData, int delayInSeconds) throws InstantiationException, IllegalAccessException {
		SlowExecutionJob job = mapAliasJob.get(alias).newInstance();
		job.setInputData(jobData);
		executionService.schedule(job, 10, TimeUnit.SECONDS);
	}
}
