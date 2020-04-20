using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using LibGit2Sharp;

namespace ExperienceMeasureCSharp
{
    class Program
    {
        const string pathWithRepos = "E:/repositories/27extra"; // TODO: Change this!

        static void Main(string[] args)
        {
            string[] directories = Directory.GetDirectories(pathWithRepos);
            Dictionary<string, long> repoMeanValues = new Dictionary<string, long>();

            foreach (string directory in directories) {
                Console.WriteLine(directory);

                using (var repo = new Repository(directory))
                {
                    HashSet<string> uniqueAuthors = new HashSet<string>();
                    var filter = new CommitFilter()
                    {
                        SortBy = CommitSortStrategies.Reverse
                    };

                    // Get all unique author names
                    foreach (Commit c in repo.Commits)
                    {
                        if (uniqueAuthors.Add(c.Author.Name.ToString()))
                        {
                            Console.WriteLine(string.Format("Author: {0}", c.Author.Name));
                        }
                    }

                    // Get the days between commits and the count the amount of authors eligible to be counted
                    long totalDays = 0;
                    long totalAuthorsConsidered = 0;
                    foreach (string s in uniqueAuthors)
                    {
                        Commit lastCommit = repo.Commits.Where(c => c.Author.Name == s).First();
                        Commit firstCommit = repo.Commits.QueryBy(filter).Where(c => c.Author.Name == s).First();

                        long daysSinceFirstCommit = (lastCommit.Author.When.ToUnixTimeSeconds() - firstCommit.Author.When.ToUnixTimeSeconds()) / 86400;

                        if (!(daysSinceFirstCommit < 1))
                        {
                            Console.WriteLine(string.Format("{0}: {1} day(s)", s, daysSinceFirstCommit));
                            totalDays += daysSinceFirstCommit;
                            totalAuthorsConsidered++;
                        }
                    }
                    long meanValue = totalDays / totalAuthorsConsidered;
                    Console.WriteLine(string.Format("Mean: {0}", meanValue));
                    repoMeanValues.Add(directory, meanValue);
                }
            }

            foreach (KeyValuePair<string, long> repoMeanPair in repoMeanValues) {
                Console.WriteLine(string.Format("{0}: {1}", repoMeanPair.Key, repoMeanPair.Value));
            }
        }
    }
}
