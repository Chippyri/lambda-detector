using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Threading;
using LibGit2Sharp;

namespace ExperienceMeasureCSharp
{
    class Program
    {
        const string PATH_WITH_REPOS = @"E:\repositories\first500repos";
        const string PATH_LAMBDA_FILE = @"E:\VisualStudioProjects\lambda-detector\out\build\x64-Release\LambdaDetector\lambda.txt";
        const string PATH_NO_LAMBDA_FILE = @"E:\VisualStudioProjects\lambda-detector\out\build\x64-Release\LambdaDetector\nolambda.txt";
        const string PATH_OUTPUT_LAMBDA = @"E:\VisualStudioProjects\lambda-detector\out\build\x64-Release\LambdaDetector\experience_lambda.txt";
        const string PATH_OUTPUT_NO_LAMBDA = @"E:\VisualStudioProjects\lambda-detector\out\build\x64-Release\LambdaDetector\experience_nolambda.txt";

        static ConcurrentQueue<string> repoNames = new ConcurrentQueue<string>();
        static ConcurrentDictionary<string, long> repoMeanValues = new ConcurrentDictionary<string, long>();

        static CommitFilter reverseFilter = new CommitFilter()
        {
            SortBy = CommitSortStrategies.Reverse
        };

        private static void EvaluateRepositories(string inputFile, string outputFile) {
            // Read all lines into a queue
            using (System.IO.StreamReader file = new System.IO.StreamReader(inputFile))
            {
                string line;
                while ((line = file.ReadLine()) != null)
                {
                    repoNames.Enqueue(line);
                }
            }

            // Not sure if the most optimal setup, but it worked!
            Thread t1 = new Thread(new ThreadStart(ThreadProc));
            Thread t2 = new Thread(new ThreadStart(ThreadProc));
            Thread t3 = new Thread(new ThreadStart(ThreadProc));
            Thread t4 = new Thread(new ThreadStart(ThreadProc));
            Thread t5 = new Thread(new ThreadStart(ThreadProc));
            Thread t6 = new Thread(new ThreadStart(ThreadProc));

            t1.Start();
            t2.Start();
            t3.Start();
            t4.Start();
            t5.Start();
            t6.Start();

            t1.Join();
            t2.Join();
            t3.Join();
            t4.Join();
            t5.Join();
            t6.Join();

            // Write results to file after everything is done
            using (System.IO.StreamWriter file = new System.IO.StreamWriter(outputFile))
            {
                file.WriteLine("repo,mean_value");  // CSV-format
                foreach (KeyValuePair<string, long> repoMeanPair in repoMeanValues)
                {
                    file.WriteLine(string.Format("{0},{1}", repoMeanPair.Key, repoMeanPair.Value));
                }
            }
        }

        private static long CalculateRepoMeanExperience(string pathToSpecificRepo)
        {
            using (var repo = new Repository(pathToSpecificRepo))
            {
                Console.WriteLine(pathToSpecificRepo);

                // Get all unique author names from all commits
                HashSet<string> uniqueAuthors = new HashSet<string>();
                foreach (Commit c in repo.Commits)
                {
                    uniqueAuthors.Add(c.Author.Name.ToString());
                }

                // Calculate experience for each author and calculate a mean sum for the repository
                long totalDays = 0;
                long totalAuthorsConsidered = 0;

                // Get first and last commit for each eligible author and add it up
                foreach (string author in uniqueAuthors)
                {
                    Commit lastCommit = repo.Commits.Where(c => c.Author.Name == author).First();
                    Commit firstCommit = repo.Commits.QueryBy(reverseFilter).Where(c => c.Author.Name == author).First();

                    long daysSinceFirstCommit = (lastCommit.Author.When.ToUnixTimeSeconds() - firstCommit.Author.When.ToUnixTimeSeconds()) / 86400;

                    if (!(daysSinceFirstCommit < 1))
                    {
                        totalDays += daysSinceFirstCommit;
                        totalAuthorsConsidered++;
                    }
                }

                long meanValue = 0;
                if (!(totalAuthorsConsidered == 0)) // Prevents division by zero
                {
                    meanValue = totalDays / totalAuthorsConsidered;
                }
                
                Console.WriteLine(string.Format("Mean: {0}", meanValue));
                return meanValue;
            }
        }

        public static void ThreadProc()
        {
            string repoName;
            while (repoNames.TryDequeue(out repoName))
            {
                Console.WriteLine(string.Format("{0}: {1}", Thread.CurrentThread.ManagedThreadId, repoName));
                Thread.Sleep(0);
                long meanValue = CalculateRepoMeanExperience(PATH_WITH_REPOS + @"\" + repoName);
                repoMeanValues.TryAdd(repoName, meanValue);
            }
        }

        static void Main(string[] args)
        {
            // Example use, remember to change the vars:
            EvaluateRepositories(PATH_LAMBDA_FILE, PATH_OUTPUT_LAMBDA);
            // EvaluateRepositories(PATH_NO_LAMBDA_FILE, PATH_OUTPUT_NO_LAMBDA);
        }
    }
}
